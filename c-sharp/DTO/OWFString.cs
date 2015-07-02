using System;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    /// Abstracts an OWF string using the built-in C# string class.
    /// OWF strings are null-terminated UTF-8 strings.
    /// </summary>
    public class OWFString : OWFObject {
        /// <summary>
        /// Memoized string size and padding size.
        /// </summary>
        private UInt32 _stringSize, _paddingSize;

        /// <summary>
        /// The actual string.
        /// </summary>
        private readonly string _str;

        /// <summary>
        /// Initializes an empty OWFString.
        /// </summary>
        public OWFString() {
            this._stringSize = this._paddingSize = UInt32.MaxValue;
            this._str = "";
        }

        /// <summary>
        /// Initializes an OWFString with the specified contents.
        /// </summary>
        /// <param name="str"></param>
        public OWFString(string str) {
            this._stringSize = this._paddingSize = UInt32.MaxValue;
            this._str = str;
        }

        /// <summary>
        /// Returns the value of this string.
        /// </summary>
        public string Value
        {
            get { return this._str; }
        }

        /// <summary>
        /// Returns the size in bytes of a null-terminated representation of this string.
        /// If the string is empty, this will return 0.
        /// Otherwise, it will return Encoding.UTF8.GetByteCount on the value, plus 1 for the null
        /// terminator.
        /// </summary>
        /// <returns>The string size in bytes.</returns>
        public UInt32 GetStringSizeInBytes() {
            if (this._stringSize == UInt32.MaxValue) {
                this._stringSize = (this.Value.Length == 0) ? 0 : ((uint)(Encoding.UTF8.GetByteCount(this.Value)) + 1);
            }
            return this._stringSize;
        }

        /// <summary>
        /// Returns the number of padding bytes required to make this string a multiple of 32 bits in length.
        /// </summary>
        /// <returns>The number of padding bytes.</returns>
        public UInt32 GetPaddingSizeInBytes() {
            if (this._paddingSize == UInt32.MaxValue) {
                var stringSize = this.GetStringSizeInBytes();
                var tmp = stringSize % 4;
                this._paddingSize = tmp == 0 ? 0 : 4 - tmp;
            }

            return this._paddingSize;
        }

        protected override UInt32 ComputeSizeInBytes() {
            var stringSize = this.GetStringSizeInBytes();
            var paddingSize = this.GetPaddingSizeInBytes();
            return sizeof(UInt32) + stringSize + paddingSize;
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            if (o is OWFString) {
                return this.Equals((OWFString)o);
            }
            if (o is string) {
                return this.Equals((string)o);
            }
            return false;
        }

        public bool Equals(OWFString other) {
            return this.Value.Equals(other.Value);
        }

        public bool Equals(string other) {
            return this.Value.Equals(other);
        }

        public override int GetHashCode() {
            return this.Value.GetHashCode();
        }
    }
}