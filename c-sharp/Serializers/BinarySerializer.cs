using System;
using System.Collections.Generic;
using System.Text;
using OWF.DTO;
using System.IO;

namespace OWF.Serializers
{
    public static class BinarySerializer
    {        

        public static void writeOWFString(BinaryWriter bw, string str)
        {
            checked
            {
                UInt32 strsize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(str));
                UInt32 padding = strsize % 4;

                // write length
                UInt32 fullSize = checked((strsize + padding));
                if (BitConverter.IsLittleEndian)
                {
                    byte[] sizeBytes = BitConverter.GetBytes(fullSize); 
                    Array.Reverse(sizeBytes);
                    bw.Write(sizeBytes);
                }
                else
                {
                    bw.Write(fullSize);
                }


                // write string (this works becuase the Binary Writer is UTF8 encoded)
                bw.Write(str);

                // write any padding
                while (padding > 0)
                {
                    bw.Write( (byte) 0 );
                    padding--;
                }
            }
        }

        public static void writeOWFDoubles( BinaryWriter bw, double [] values )
        {            
            if (BitConverter.IsLittleEndian)
            {
                for (var i = 0; i < values.Length; i++)
                {
                    byte[] valueBytes = BitConverter.GetBytes(values[i]);
                    Array.Reverse(valueBytes);
                    bw.Write(valueBytes);
                }
            }
            else
            {
                for (var i = 0; i < values.Length; i++)
                {
                    bw.Write(values[i]);
                }
            }
        }

        public static void writeOWFTimestamp(BinaryWriter bw, DateTime time)
        {
            // filetime is 100ns increments from 1601...we use a different epoch, so we offset.
            Int64 unixEpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTime();
            Int64 time64 = time.ToFileTime();
            Int64 adjTime = time64 - unixEpochOffset;

            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(adjTime);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else
            {
                bw.Write(adjTime);
            }
        }

        public static void writeHeader( BinaryWriter bw, UInt32 length ){
            byte[] magic = {0x4f, 0x57, 0x46, 0x31 };
            bw.Write( magic );
            bw.Write(length);
        }

        public static void writeChannel(BinaryWriter bw, Channel channel)
        {
            bw.Write(channel.getSizeInBytes());
        }

        public static byte[] convert(Package package)
        {
            using (MemoryStream mem = new MemoryStream())
            {
                using (BinaryWriter bw = new BinaryWriter(mem, Encoding.UTF8))
                {
                    writeHeader(bw, package.getSizeInBytes());

                    foreach( var channel in package.Channels) {
                        writeChannel(bw, channel);
                    }
                }
                return mem.ToArray();
            }            
        }
    }
}
