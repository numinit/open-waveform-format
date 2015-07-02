using System;

namespace OWF.DTO {
    public abstract class OWFObject {
        /// <summary>
        /// The OWF1 magic header.
        /// </summary>
        public const UInt32 Magic = 0x4f574631U;

        /// <summary>
        /// This object's cached size.
        /// </summary>
        private UInt32 _size;

        /// <summary>
        /// Initializes this OWFObject. Should only be called from subclasses.
        /// </summary>
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