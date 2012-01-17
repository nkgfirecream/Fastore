using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime;

namespace Fastore.Core
{
	/// <summary> Similar to Nullable of T, but allows reference types. </summary>
	public struct Optional<T> 
	{
		private bool hasValue;
		internal T value;

		public static readonly Optional<T> Null = new Optional<T>();

		public bool HasValue
		{
			get
			{
				return this.hasValue;
			}
		}
		
		public T Value
		{
			[TargetedPatchingOptOut("Performance critical to inline across NGen image boundaries")]
			get
			{
				if (!this.HasValue)
				{
					throw new InvalidOperationException("Attempt to access an optional variable which has no value.");
				}
				return this.value;
			}
		}

		[TargetedPatchingOptOut("Performance critical to inline across NGen image boundaries")]
		public T GetValueOrDefault()
		{
			return this.value;
		}

		public T GetValueOrDefault(T defaultValue)
		{
			if (!this.HasValue)
			{
				return defaultValue;
			}
			return this.value;
		}

		public override bool Equals(object other)
		{
			if (other is Optional<T>)
				return (Optional<T>)other == this;
			else
				return base.Equals(other);
		}

		public override int GetHashCode()
		{
			if (!this.HasValue)
			{
				return 0;
			}
			return this.value.GetHashCode();
		}

		public override string ToString()
		{
			if (!this.HasValue)
			{
				return "";
			}
			return this.value.ToString();
		}

		public static implicit operator Optional<T>(T value)
		{
			return new Optional<T> { hasValue = true, value = value };
		}

		public static explicit operator T(Optional<T> value)
		{
			return value.Value;
		}

		public static bool operator ==(Optional<T> left, Optional<T> right)
		{
			if (left.hasValue)
			{
				if (right.hasValue)
					return left.Equals(right.value);
				else
					return false;
			}
			else
				return !right.hasValue;
		}

		public static bool operator !=(Optional<T> left, Optional<T> right)
		{
			return !(left == right);
		}
	}
}
