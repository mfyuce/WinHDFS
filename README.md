# WinHDFS
HDFS file access enabler for .Net and native codes. 

# Original code  

https://code.msdn.microsoft.com/windowsdesktop/Hadoop-Net-HDFS-File-Access-18e9bbee
 https://blogs.msdn.microsoft.com/carlnol/2013/02/08/hadoop-net-hdfs-file-access/
.

# To Be able to Build;

-Visual Studio
-JDK home
-Hadoop installation 
-Arrange your include directories accordingly

# Sample

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