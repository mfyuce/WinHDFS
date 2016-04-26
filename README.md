# WinHDFS
HDFS file access for .Net and native code on Windows System. 

# Original code  

https://code.msdn.microsoft.com/windowsdesktop/Hadoop-Net-HDFS-File-Access-18e9bbee

https://blogs.msdn.microsoft.com/carlnol/2013/02/08/hadoop-net-hdfs-file-access/


# To Be able to Build;

*Visual Studio

*JDK home (JAVA_HOME)

*Hadoop installation (HADOOP_HOME)

*Arrange your include directories accordingly (JAVA_HOME\Include etc.)

# Sample
  {
  
    using (HdfsFileSystem hdfsSystem = HdfsFileSystem.Connect("<host>", 9000,"hadoop"))
     {
         Console.WriteLine("TESTING HdfsFileStream:");
         
         using (HdfsFileStream file = hdfsSystem.OpenFileStream(filename, HdfsFileAccess.Write, chunksize))
         {
             file.Write(dataBytes, 0, data.Length);
             file.WriteByte((byte)47);
             file.Flush();
         }
   
         // Ensure you can read the data
         using (HdfsFileStream file = hdfsSystem.OpenFileStream(filename, HdfsFileAccess.Read))
         { 
             byte[] newDataBytes = new byte[dataLen];
             file.Read(newDataBytes, 0, newDataBytes.Length);
             //..
         } 
     } 
     
   }
  
