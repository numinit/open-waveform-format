using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using OWF.DTO;

namespace OWF.Serializers {
    /// <summary>
    /// A class representing state for unpacking OWF data.
    /// </summary>
    public class BinaryUnpacker {
        /// <summary>
        /// A BinaryReader for this BinaryUnpacker.
        /// </summary>
        private BinaryReader _br;

        private UInt32 _segmentLength;

        /// <summary>
        /// The maximum size we're willing to allocate.
        /// </summary>
        private readonly UInt32 _maxAlloc;

        /// <summary>
        /// Initializes this BinaryUnpacker given a maximum single allocation size
        /// of 1 megabyte. If you want to unpack bigger packets, use the other constructor
        /// and specify a size.
        /// </summary>
        public BinaryUnpacker() : this(0x100000) {
        }

        /// <summary>
        /// Initializes this BinaryUnpacker given a maximum single allocation size.
        /// </summary>
        /// <param name="maxAlloc">The maximum size for individual memory allocations.</param>
        public BinaryUnpacker(UInt32 maxAlloc) {
            this._maxAlloc = maxAlloc;
        }

        /// <summary>
        /// A delegate type for reader operations
        /// </summary>
        /// <typeparam name="T">The type to return</typeparam>
        /// <returns>An object of type T</returns>
        private delegate T ReaderDelegate<out T>();

        /// <summary>
        /// The top-level unwrapper function. Should be passed as a delegate.
        /// </summary>
        /// <returns>A list of channels</returns>
        private List<OWFChannel> UnwrapTop() {
            return this.UnwrapNestedMulti(this.ReadOWFChannel);
        }

        /// <summary>
        /// Reads an OWFChannel from the BinaryReader. Should be passed as a delegate.
        /// </summary>
        /// <returns>An OWFChannel</returns>
        private OWFChannel ReadOWFChannel() {
            var id = this.Unwrap(this.ReadOWFString);
            return new OWFChannel(id, this.UnwrapNestedMulti(this.ReadOWFNamespace));
        }

        /// <summary>
        /// Reads an OWFNamespace from the BinaryReader. Should be passed as a delegate.
        /// </summary>
        /// <returns>An OWFNamesapce</returns>
        private OWFNamespace ReadOWFNamespace() {
            var t0 = this.ReadS64();
            var dt = this.ReadU64();
            var id = this.Unwrap(this.ReadOWFString);
            var signals = this.Unwrap(this.ReadOWFSignals);
            var events = this.Unwrap(this.ReadOWFEvents);
            var alarms = this.Unwrap(this.ReadOWFAlarms);
            return new OWFNamespace(id, t0, dt, signals, events, alarms);
        }

        /// <summary>
        /// Reads an OWFSignal from the BinaryReader.
        /// </summary>
        /// <returns>An OWFSignal</returns>
        private OWFSignal ReadOWFSignal() {
            var id = this.Unwrap(this.ReadOWFString);
            var unit = this.Unwrap(this.ReadOWFString);
            var rawSamples = this.Unwrap(this.ReadSegment);
            if (rawSamples.Length % sizeof(double) != 0) {
                throw new UnpackError("length of sample array is not {0}-byte aligned (got {1} bytes)", sizeof(double), rawSamples.Length);
            }

            var samples = new double[rawSamples.Length / sizeof(double)];
            if (BitConverter.IsLittleEndian) {
                for (var i = 0; i < rawSamples.Length; i += sizeof(double)) {
                    Array.Reverse(rawSamples, i, sizeof(double));
                    samples[i / sizeof(double)] = BitConverter.ToDouble(rawSamples, i);
                }
            }
            else {
                for (var i = 0; i < rawSamples.Length; i += sizeof(double)) {
                    samples[i / sizeof(double)] = BitConverter.ToDouble(rawSamples, i);
                }
            }
            return new OWFSignal(id, unit, samples);
        }

        /// <summary>
        /// Reads multiple OWFSignal objects from the BinaryReader. Should be passed as a delegate.
        /// </summary>
        /// <returns>A list of OWFSignal objects</returns>
        private List<OWFSignal> ReadOWFSignals() {
            return this.UnwrapMulti(this.ReadOWFSignal);
        }

        /// <summary>
        /// Reads an OWFEvent from the BinaryReader.
        /// </summary>
        /// <returns>An OWFEvent</returns>
        private OWFEvent ReadOWFEvent() {
            var t0 = this.ReadS64();
            var message = this.Unwrap(this.ReadOWFString);
            return new OWFEvent(message, t0);
        }

        /// <summary>
        /// Reads multiple OWFEvent objects from the BinaryReader. Should be passed as a delegate.
        /// </summary>
        /// <returns>A list of OWFEvent objects</returns>
        private List<OWFEvent> ReadOWFEvents() {
            return this.UnwrapMulti(this.ReadOWFEvent);
        }

        /// <summary>
        /// Reads an OWFAlarm object from the BinaryReader.
        /// </summary>
        /// <returns>An OWFAlarm</returns>
        private OWFAlarm ReadOWFAlarm() {
            var t0 = this.ReadS64();
            var dt = this.ReadU64();
            var levelVolume = this.ReadU32();
            var level = (byte)((levelVolume & 0xff000000) >> 24);
            var volume = (byte)((levelVolume & 0x00ff0000) >> 16);
            var type = this.Unwrap(this.ReadOWFString);
            var message = this.Unwrap(this.ReadOWFString);
            return new OWFAlarm(type, message, t0, dt, level, volume);
        }

        /// <summary>
        /// Reads multiple OWFAlarm objects from the BinaryReader. Should be passed as a delegate.
        /// </summary>
        /// <returns>A list of OWFAlarm objects</returns>
        private List<OWFAlarm> ReadOWFAlarms() {
            return this.UnwrapMulti(this.ReadOWFAlarm);
        }

        /// <summary>
        /// Reads a length.
        /// Throws an UnpackError if the length is not 4-byte aligned.
        /// </summary>
        /// <returns>A length</returns>
        private UInt32 ReadLength() {
            var val = this.ReadU32();
            if (val % sizeof(UInt32) != 0) {
                throw new UnpackError("length was not {0}-byte aligned (got {1} bytes)", sizeof(UInt32), val);
            }
            return val;
        }

        /// <summary>
        /// Reads a 32-bit integer.
        /// </summary>
        /// <returns>The 32-bit integer</returns>
        private UInt32 ReadU32() {
            var val = this._br.ReadUInt32();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }

            this.MarkRead(sizeof(UInt32));
            return val;
        }

        /// <summary>
        /// Reads a 64-bit unsigned integer.
        /// </summary>
        /// <returns>The 64-bit unsigned integer</returns>
        private UInt64 ReadU64() {
            var val = this._br.ReadUInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt64(valueBytes, 0);
            }

            this.MarkRead(sizeof(UInt64));
            return val;
        }

        /// <summary>
        /// Reads a 64-bit signed integer.
        /// </summary>
        /// <returns>The 64-bit signed integer</returns>
        private Int64 ReadS64() {
            var val = this._br.ReadInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToInt64(valueBytes, 0);
            }

            this.MarkRead(sizeof(Int64));
            return val;
        }

        /// <summary>
        /// Reads the remainder of the current segment into a byte array. Should be passed as a delegate.
        /// If the length is greater than MaxAlloc, an UnpackError is thrown.
        /// </summary>
        /// <returns>The remainder of the current segment, as a byte array</returns>
        private byte[] ReadSegment() {
            if (this._segmentLength > this._maxAlloc) {
                throw new UnpackError("allocated size was greater than max ({0} > {1})", this._segmentLength, this._maxAlloc);
            }
            else if (this._segmentLength == 0) {
                return new byte[] {};
            }
            else {
                // Read the rest of the segment, and mark the whole thing as read
                byte[] ret = this._br.ReadBytes(checked((Int32)this._segmentLength));
                this.MarkRead(this._segmentLength);
                return ret;
            }
        }

        /// <summary>
        /// Reads an OWFString from the current segment. Should be passed as a delegate.
        /// If the string is longer than MaxAlloc or is not null-terminated, an UnpackError is thrown.
        /// </summary>
        /// <returns>An OWFString</returns>
        public OWFString ReadOWFString() {
            OWFString ret;
            if (this._segmentLength > 0) {
                var rawString = this.ReadSegment();

                // Make sure that the string is null-terminated
                if (rawString[rawString.Length - 1] != 0) {
                    throw new UnpackError("string was not NULL-terminated");
                }

                // Get the actual length of this string (cutting out null padding)
                int i = 0;
                while (rawString[i] != 0) {
                    i++;
                }

                // Get a UTF-8 view of the part before the null bytes
                ret = new OWFString(Encoding.UTF8.GetString(rawString, 0, i));
            }
            else {
                ret = new OWFString();
            }

            return ret;
        }

        /// <summary>
        /// Unwraps a length-wrapped object, and calls a delegate.
        /// </summary>
        /// <typeparam name="T">The type to return.</typeparam>
        /// <param name="d">The delegate to call.</param>
        /// <returns>The object resulting from the call to the delegate.</returns>
        private T Unwrap<T>(ReaderDelegate<T> d) {
            UInt32 oldLength = this._segmentLength;
            try {
                // Unwrap the value
                UInt32 length = 0;
                var val = this.UnwrapValue(d, ref (length));

                // Restore the length, and mark that many bytes as read
                this._segmentLength = oldLength;
                this.MarkRead(length);

                // Return the unwrapped value
                return val;
            }
            catch (UnpackError) {
                this._segmentLength = oldLength;
                throw;
            }
        }

        /// <summary>
        /// Unwraps multiple length-wrapped objects. Should be passed as a delegate.
        /// </summary>
        /// <typeparam name="T">The type of each object</typeparam>
        /// <param name="d">The delegate</param>
        /// <returns>A list of the objects</returns>
        private List<T> UnwrapNestedMulti<T>(ReaderDelegate<T> d) {
            var ret = new List<T>();
            while (this._segmentLength > 0) {
                ret.Add(this.Unwrap(d));
            }
            return ret;
        }

        /// <summary>
        /// Unwraps multiple non-length-wrapped objects. Should be passed as a delegate.
        /// </summary>
        /// <typeparam name="T">The type of each object</typeparam>
        /// <param name="d">The delegate</param>
        /// <returns>A list of the objects</returns>
        private List<T> UnwrapMulti<T>(ReaderDelegate<T> d) {
            var ret = new List<T>();
            while (this._segmentLength > 0) {
                ret.Add(d());
            }
            return ret;
        }

        /// <summary>
        /// Unwraps a value, storing the length in bytes to outputLength.
        /// Throws an UnpackError if there was any trailing data, or if any
        /// problems were encountered unpacking nested objects.
        /// </summary>
        /// <typeparam name="T">The type to read</typeparam>
        /// <param name="d">The delegate</param>
        /// <param name="outputLength">The length we read</param>
        /// <returns>The object resulting from the unwrap</returns>
        private T UnwrapValue<T>(ReaderDelegate<T> d, ref UInt32 outputLength) {
            // Read the length
            var length = this.ReadLength();

            // Store the old length, and push the new length
            this._segmentLength = length;

            // Call the delegate
            var ret = d();

            // Ensure that we have no trailing bytes
            if (this._segmentLength > 0) {
                throw new UnpackError("trailing data when reading segment: {0} bytes", this._segmentLength);
            }

            // Store how many bytes we read, and return the unwrapped value
            outputLength = length + sizeof(UInt32);
            return ret;
        }

        /// <summary>
        /// Marks `bytes` as read.
        /// Will throw an UnpackError on underflow.
        /// </summary>
        /// <param name="bytes">The number of bytes</param>
        private void MarkRead(UInt32 bytes) {
            this._segmentLength = Sub32(this._segmentLength, bytes);
        }

        /// <summary>
        /// Performs a safe 32-bit addition.
        /// Will throw an UnpackError if the addition overflows.
        /// </summary>
        /// <param name="a">The first int</param>
        /// <param name="b">The second int</param>
        /// <returns>a + b</returns>
        private static UInt32 Add32(UInt32 a, UInt32 b) {
            try {
                return checked(a + b);
            }
            catch (System.OverflowException e) {
                throw new UnpackError("unsigned 32-bit addition overflow ({0} + {1})", e, a, b);
            }
        }

        /// <summary>
        /// Performs a safe 32-bit subtraction.
        /// Will throw an UnpackError if the subtraction underflows.
        /// </summary>
        /// <param name="a">The first int</param>
        /// <param name="b">The second int</param>
        /// <returns>a - b</returns>
        private static UInt32 Sub32(UInt32 a, UInt32 b) {
            if (a < b) {
                throw new UnpackError("unsigned 32-bit subtraction underflow ({0} - {1})", a, b);
            }
            return checked(a - b);
        }

        /// <summary>
        /// Performs a safe 32-bit multiply.
        /// Will throw an UnpackError if the multiply overflows.
        /// </summary>
        /// <param name="a">The first int</param>
        /// <param name="b">The second int</param>
        /// <returns>a * b</returns>
        private static UInt32 Mul32(UInt32 a, UInt32 b) {
            try {
                return checked(a * b);
            }
            catch (System.OverflowException e) {
                throw new UnpackError("unsigned 32-bit multiplication overflow ({0} * {1})", e, a, b);
            }
        }

        /// <summary>
        /// Converts a stream to an OWFPackage.
        /// Throws an UnpackError if there was a problem unpacking the stream.
        /// </summary>
        /// <param name="str">The stream</param>
        /// <returns>An OWFPackage</returns>
        public OWFPackage Convert(Stream str) {
            lock (this) {
                using (this._br = new BinaryReader(str, Encoding.UTF8, true)) {
                    // Read the header
                    UInt32 length = sizeof(UInt32);
                    this._segmentLength = length;
                    var magic = this.ReadU32();
                    if (magic != OWFObject.Magic) {
                        throw new UnpackError("invalid magic header: {0:X8}", magic);
                    }

                    // Deserialize the OWFPackage
                    this._segmentLength = length;
                    return new OWFPackage(this.UnwrapValue(this.UnwrapTop, ref (length)));
                }
            }
        }

        /// <summary>
        /// Converts a byte array to an OWFPackage.
        /// Throws an UnpackError if there was a problem unpacking the stream.
        /// </summary>
        /// <param name="package">The package</param>
        /// <returns>An OWFPackage</returns>
        public OWFPackage Convert(byte[] package) {
            using (var mem = new MemoryStream(package, false)) {
                return this.Convert(mem);
            }
        }

        /// <summary>
        /// Converts a stream to an OWFPackage.
        /// Throws an UnpackError if there was a problem unpacking the stream.
        /// </summary>
        /// <param name="str">The stream</param>
        /// <returns>An OWFPackage</returns>
        public static OWFPackage Unpack(Stream str) {
            return new BinaryUnpacker().Convert(str);
        }

        /// <summary>
        /// Converts a byte array to an OWFPackage.
        /// Throws an UnpackError if there was a problem unpacking the stream.
        /// </summary>
        /// <param name="package">The package</param>
        /// <returns>An OWFPackage</returns>
        public static OWFPackage Unpack(byte[] package) {
            return new BinaryUnpacker().Convert(package);
        }

        /// <summary>
        /// An error thrown if there is a problem unpacking the OWF.
        /// </summary>
        [Serializable]
        public class UnpackError : Exception {
            public UnpackError(String message, params Object[] args) : base(String.Format(message, args)) {}
            public UnpackError(String message, Exception cause, params Object[] args) : base(String.Format(message, args), cause) {}
        }
    }
}