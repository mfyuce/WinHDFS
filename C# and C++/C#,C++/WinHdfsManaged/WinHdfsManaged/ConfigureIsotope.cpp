#include "stdafx.h"
#include "ConfigureIsotope.h"
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <string>
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Linq;

namespace
{
    // Gets an environment variable from the CRT
    // This is a copy of the process' "environment" obtained using GetEnvironmentStrings
    // and is cached as soon as the CRT is loaded.    
    System::String^ GetCRTEnvironmentVariable(System::String^ envVariable)
    {
        auto envVariableString = msclr::interop::marshal_as<std::wstring> (envVariable);

        size_t requiredLength;
        _wgetenv_s(&requiredLength, nullptr, 0, envVariableString.c_str());

        if (requiredLength == 0)
        {
            return System::String::Empty;
        }
        
        std::vector<wchar_t> envValuePtr(requiredLength);
        _wgetenv_s(&requiredLength, envValuePtr.data(), requiredLength, envVariableString.c_str());
        return msclr::interop::marshal_as<System::String^> (envValuePtr.data());
    }

    void SetCRTEnvironemtVariable(System::String^ envVariable, System::String^ value)
    {
        auto envVariableString = msclr::interop::marshal_as<std::wstring> (envVariable);
        auto valueString       = msclr::interop::marshal_as<std::wstring> (value);
        _wputenv_s(envVariableString.c_str(), valueString.c_str());
    }

    // Adds a directory to the DLL search path
    void AddDllPath(System::String^ path)
    {
        if (!System::IO::Directory::Exists(path))
        {
            throw gcnew System::IO::FileNotFoundException(path + " was not found.");
        }
        ::SetDllDirectoryW(msclr::interop::marshal_as<std::wstring>(path).c_str());
    }
}

namespace Microsoft
{
    namespace Hdfs
    { 

        void ConfigureIsotope::Setup()
        {
            if (!_isSetup)
            {
                FindIsotope();
                FindJava();
                FindJars();
                SetupPath();
                SetupJavaHome();
                SetupClassPath();
                _isSetup = true;
            }
        }

        void ConfigureIsotope::FindIsotope()
        {
            _hadoopHome = System::Environment::GetEnvironmentVariable("HADOOP_HOME") + "\\";
            if (_hadoopHome == nullptr)
            {
                throw gcnew System::IO::FileNotFoundException("HADOOP_HOME is not defined, please ensure Isotope is installed correctly.");
            }

			_isotopeHome = _hadoopHome;// System::IO::Path::GetDirectoryName(_hadoopHome);
        }

        void ConfigureIsotope::FindJava()
        {
			_javaRuntime = FindJre(System::Environment::GetEnvironmentVariable("JAVA_HOME"));
            _javaHome = System::IO::Path::GetDirectoryName(_javaRuntime);
        }

        void ConfigureIsotope::FindJars()
        {
            System::String^ hadoopLibs =  _hadoopHome ;

           // System::String^ hadoopCore              = FindFile(_hadoopHome, "hadoop-core-*.jar", "Base Hadoop.");
			auto hadoopCore = FindFiles(_hadoopHome, "hadoop-*.jar", "Base Hadoop.");
			auto commonsLoggingJar = FindFiles(hadoopLibs, "commons*.jar", "Apache Commons Logging.");
			auto commonsConfigurationJar = FindFiles(hadoopLibs, "slf4j-*.jar", "Apache Commons Configuration.");
			auto commonsLangJar = FindFiles(hadoopLibs, "log4j*.jar", "Apache Commons Language."); 
			auto guavaJar = FindFiles(hadoopLibs, "guava*.jar", "Apache Commons Language.");
			auto awsJar = FindFiles(hadoopLibs, "aws*.jar", "Apache Commons Language.");
			auto htraceJar = FindFiles(hadoopLibs, "htrace*.jar", "Apache Commons Language.");
			auto servletapiJar = FindFiles(hadoopLibs, "servlet-api*.jar", "Apache Commons Language.");
			auto protobufJar = FindFiles(hadoopLibs, "protobuf*.jar", "Apache Commons Language.");

            _jarFiles = gcnew System::Collections::Generic::List<System::String^>();
            _jarFiles->AddRange(hadoopCore);
			_jarFiles->AddRange(commonsLoggingJar);
			_jarFiles->AddRange(commonsConfigurationJar);
			 _jarFiles->AddRange(commonsLangJar); 
			_jarFiles->AddRange(guavaJar);
			_jarFiles->AddRange(awsJar);
			_jarFiles->AddRange(htraceJar);
			_jarFiles->AddRange(servletapiJar);
			_jarFiles->AddRange(protobufJar);
        }

        System::String^ ConfigureIsotope::FindJre(System::String^ dir)
        {        
            auto dirs = System::Linq::Enumerable::LastOrDefault(System::IO::Directory::EnumerateDirectories(dir, "jre", System::IO::SearchOption::AllDirectories));

            if (dirs == nullptr)
            {
                throw gcnew System::IO::FileNotFoundException("JRE could not be found, please ensure Isotope is installed correctly.");
            }
            return dirs;
        }

		array<System::String^>^ ConfigureIsotope::FindFiles(System::String^ path, System::String^ pattern)
        {
            return FindFiles(path, pattern, pattern);
        }

		array<System::String^>^ ConfigureIsotope::FindFiles(System::String^ path, System::String^ pattern, System::String^ description)
        {        
            auto file = System::IO::Directory::EnumerateFiles(path, pattern, System::IO::SearchOption::AllDirectories);
            if (file == nullptr)
            {
                throw gcnew System::IO::FileNotFoundException(description);
            }
			return System::Linq::Enumerable::ToArray(System::Linq::Enumerable::Distinct(file));
        }

        void ConfigureIsotope::SetupClassPath()
        {        
            for each (System::String^ jar in _jarFiles)
            {
                SetEnvironmentVariable("CLASSPATH", jar, EnvironmentVariableMode::APPEND, false);
            }
        }

        void ConfigureIsotope::SetupJavaHome()
        {
            SetEnvironmentVariable("JAVA_HOME", _javaHome, EnvironmentVariableMode::OVERWRITE, false);
        }

        void ConfigureIsotope::SetupPath()
        {
            AddDllPath(System::IO::Path::Combine(_javaRuntime,"bin"));
            AddDllPath(System::IO::Path::Combine(_javaRuntime,"bin", "server"));
        }

        System::String^ ConfigureIsotope::Quotify(System::String^ value)
        {
            return (value->StartsWith("\"") && value->EndsWith("\"")) ? value : ("\""+value+"\"");        
        }

        void ConfigureIsotope::SetEnvironmentVariable(System::String^ env, System::String^ newValue, EnvironmentVariableMode mode, bool quotify)
        {        
            if (quotify)
            {
                newValue = Quotify(newValue);
            }
        
            auto value = GetCRTEnvironmentVariable(env);

            if (value == System::String::Empty || mode == EnvironmentVariableMode::OVERWRITE)
            {
                value = newValue;
            }
            else if (mode == EnvironmentVariableMode::PREPEND)
            {
                value = newValue + ";" + value;
            }
            else
            {
                value = value + ";" + newValue;
            }

            SetCRTEnvironemtVariable(env, value);        
        }

        void ConfigureIsotope::SetEnvironmentVariable(System::String^ env, System::String^ newValue)
        {
            SetEnvironmentVariable(env, newValue, EnvironmentVariableMode::APPEND, true);
        }
    }
}
