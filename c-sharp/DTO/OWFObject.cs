using System;

namespace OWF.DTO {
    public abstract class OWFObject {
        private UInt32 _size;

        protected OWFObject() {
            this._size = UInt32.MaxValue;
        }

        /// <summary>
        /// Gets the size in bytes of this object.
        /// </summary>
        /// <returns></returns>
        public UInt32 GetSizeInBytes() {
            if (this._size == UInt32.MaxValue) {
                this._size = this.ComputeSizeInBytes();
            }

            return this._size;
        }

        /// <summary>
        /// Computes the size, in bytes, of this object.
        /// </summary>
        /// <returns></returns>
        protected abstract UInt32 ComputeSizeInBytes();
    }
}