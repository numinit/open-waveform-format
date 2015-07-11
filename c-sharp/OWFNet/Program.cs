using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.Remoting.Metadata.W3cXsd2001;
using System.Threading;
using OWF.DTO;
using OWF.Serializers;

namespace OWFNet {
    internal class Program {
        private const int ProtocolTcp = 0, ProtocolUdp = 1;

        private static void Main(string[] args) {
            if (args.Length != 4) {
                throw new ArgumentException("Usage: OWFNet <owf-file> <tcp|udp> <host> <port>");
            }

            OWFPackage owf;
            string fileString = args[0], protocolString = args[1], hostString = args[2];
            int port = int.Parse(args[3]), protocol;

            // Validate the port
            if (port < 0x0000 || port > 0xffff) {
                throw new ArgumentException("port out of range");
            }

            // Decide the protocol
            if (protocolString.Equals("tcp")) {
                protocol = ProtocolTcp;
            } else if (protocolString.Equals("udp")) {
                protocol = ProtocolUdp;
            } else {
                throw new ArgumentException("invalid protocol (use 'tcp' or 'udp')");
            }

            // Read the OWF input file. Use both methods of reading an OWF file so we can
            // have a reference copy (raw binary) and a "streamed" parsed copy for testing.
            var owfBytes = File.ReadAllBytes(fileString);
            using (var fs = File.OpenRead(fileString)) {
                owf = BinaryUnpacker.Unpack(fs);
            }

            try {
                // Connect
                if (protocol == ProtocolTcp) {
                    var client = new TcpClient(hostString, port);
                    while (true) {
                        RunTcp(client, owfBytes, owf);
                        Thread.Sleep(500);
                    }
                } else {
                    var client = new UdpClient(hostString, port);
                    while (true) {
                        RunUdp(client, owfBytes, owf);
                        Thread.Sleep(500);
                    }
                }
            } catch (Exception e) {
                Console.WriteLine("caught exception: {0}", e.ToString());
            }
        }

        private static void RunTcp(TcpClient client, byte[] owfBytes, OWFPackage owf) {
            // Pack the OWF and write it to the socket
            try {
                BinaryPacker.Pack(owf, client.GetStream());
            } catch (BinaryPacker.PackError e) {
                throw new Exception("error packing OWF", e);
            }

            // Unpack what we're about to receive from the socket
            try {
                var unpacked = BinaryUnpacker.Unpack(client.GetStream());
                if (!unpacked.Equals(owf)) {
                    Console.WriteLine("unpacked OWF from server does not equal packed OWF sent to server!");
                } else {
                    Console.WriteLine("successfully unpacked OWF: {0} bytes", unpacked.GetSizeInBytes());
                }
            } catch (BinaryUnpacker.UnpackError e) {
                Console.WriteLine("error unpacking OWF: {0}", e);
            }
        }

        private static void RunUdp(UdpClient client, byte[] owfBytes, OWFPackage owf) {
            // Pack the OWF and write it to the client
            byte[] buf;
            IPEndPoint src = null;
            try {
                buf = BinaryPacker.Pack(owf);
                client.Send(buf, buf.Length);
            } catch (BinaryPacker.PackError e) {
                throw new Exception("error packing OWF to buffer", e);
            } catch (Exception e) {
                throw new Exception("exception while sending datagram", e);
            }

            // Read a datagram from the server, and unpack it
            byte[] received = client.Receive(ref src);
            try {
                var unpacked = BinaryUnpacker.Unpack(received);
                if (!received.SequenceEqual(buf)) {
                    Console.WriteLine("binary OWF from server does not equal OWF sent to server!");
                } else if (!unpacked.Equals(owf)) {
                    Console.WriteLine("unpacked OWF from server does not equal OWF sent to server!");
                } else {
                    Console.WriteLine("successfully unpacked OWF: {0} bytes", unpacked.GetSizeInBytes());
                }
            } catch (BinaryUnpacker.UnpackError e) {
                Console.WriteLine("error unpacking OWF: {0}", e);
            }
        }
    }
}