using System;
using System.Collections;
using System.Collections.Generic;

namespace Fastore.Core
{
	/// <summary>
	/// Default values for the tunable parameters of the PartitionedList.
	/// </summary>
	public static class PartitionedListParameters
	{
		/// <summary>
		/// The default capacity of data nodes in the partitioned list.
		/// </summary>
		public static int DefaultCapacity = 100;
		
		/// <summary>
		/// The default capacity of routing nodes in the partitioned list.
		/// </summary>
		public static int DefaultFanout = 100;
		
		/// <summary>
		/// The threshold that determines how far away the currently cached scan must be from a requested index in order to request a new scan.
		/// </summary>
		/// <remarks>
		/// Internally, the partitioned list maintains a scan with the list requested index. When an index request is made, if the current
		/// scan's position is within this threshold from the requested index, the current scan will be positioned on the requested index,
		/// otherwise, a new scan will be requested on the new index.
		/// </remarks>
		public static int UseCurrentScanThreshold = 100;
	}
	
	/// <summary>
	/// Represents a list that uses a partitioned array to store its elements.
	/// </summary>
	/// <typeparam name="T">The type of elements the partitioned list will store.</typeparam>
	/// <remarks>
	/// This class maintains a list of elements using a B-Tree style index
	/// that uses the index of the element for routing information rather than
	/// the value of any data within each element.
	/// </remarks>
	public class PartitionedList<T> : IList, IList<T>
	{
		private Dictionary<T, PartitionedListDataNode<T>> _reverseIndex;

		/// <summary>
		/// The minimum fanout possible.
		/// </summary>
		/// <remarks>
		/// A fanout below this value would result in an unpartitioned tree.
		/// </remarks>
		public const int MinimumFanout = 2;
		
		/// <summary>
		/// Constructs a new partitioned list with the given capacity and fanout.
		/// </summary>
		/// <param name="capacity">The number of elements that can be stored per data node.</param>
		/// <param name="fanout">The number of routing entries that can be stored per routing node.</param>
		public PartitionedList(int capacity, int fanout)
		{
			_capacity = capacity;
			_fanout = fanout < MinimumFanout ? MinimumFanout : fanout;
			_head = new PartitionedListDataNode<T>(_capacity);
			_reverseIndex = new Dictionary<T, PartitionedListDataNode<T>>(_capacity);
			_tail = _head;
			_root = _head;
			_height = 1;
		}
		
		/// <summary>
		/// Constructs a new partitioned list with the default capacity and fanout (100).
		/// </summary>
		public PartitionedList() : this(PartitionedListParameters.DefaultCapacity, PartitionedListParameters.DefaultFanout) { }
		
		private int _height;
		/// <summary>
		/// The number of levels present in the partitioned list.
		/// </summary>
		public int Height { get { return _height; } }
		
		private int _capacity = PartitionedListParameters.DefaultCapacity;
		/// <summary>
		/// The number of elements that can be stored per data node in the partitioned list.
		/// </summary>
		public int Capacity { get { return _capacity; } }
		
		private int _fanout = PartitionedListParameters.DefaultFanout;
		/// <summary>
		/// The number of elements that can be stored per routing node in the partitioned list.
		/// </summary>
		public int Fanout { get { return _fanout; } }

		/// <summary>
		/// A reference to the first data node in the list.
		/// </summary>
		private PartitionedListDataNode<T> _head;
		
		/// <summary>
		/// A reference to the last data node in the list.
		/// </summary>
		private PartitionedListDataNode<T> _tail;
		
		/// <summary>
		/// A reference to the root node of the list.
		/// </summary>
		private PartitionedListNode _root;
		
		/// <summary>
		/// Represents a scan of the partitioned list.
		/// </summary>
		/// <typeparam name="T">The type of elements in the partitioned list.</typeparam>
		private class Scan
		{
			/// <summary>
			/// Constructs a new instance of a scan using the given path information.
			/// </summary>
			/// <remarks>
			/// The scan will initially be positioned on the element identified in the given path.
			/// </remarks>
			public Scan(PartitionedListPath path)
			{
				_absoluteIndex = path.SearchIndex;
				_currentNode = path[path.Length - 1].Node as PartitionedListDataNode<T>;
				_currentIndex = path[path.Length - 1].EntryIndex;
			}
			
			private PartitionedListDataNode<T> _currentNode;
			private int _currentIndex;

			private int _absoluteIndex;
			/// <summary>
			/// The position of the scan within the partitioned list.
			/// </summary>
			public int AbsoluteIndex { get { return _absoluteIndex; } }
			
			/// <summary>
			/// Gets or sets the element at the scan's current position.
			/// </summary>
			public T Current
			{
				get
				{
					if ((_currentNode == null) || (_absoluteIndex < 0) || (_currentIndex >= _currentNode.EntryCount))
						throw new IndexOutOfRangeException(String.Format("Index (%d) out of range.", _absoluteIndex));
					return _currentNode.Elements[_currentIndex]; 
				}
				set
				{
					if ((_currentNode == null) || (_absoluteIndex < 0) || (_currentIndex >= _currentNode.EntryCount))
						throw new IndexOutOfRangeException(String.Format("Index (%d) out of range.", _absoluteIndex));
						
					_currentNode.Elements[_currentIndex] = value;
				}
			}

			/// <summary>
			/// Moves the scan forward one element, returning true if the scan is positioned on an element within the list.
			/// </summary>
			/// <returns>True if the scan is positioned on an element within the list after the move, false otherwise.</returns>
			public bool Next()
			{
				_absoluteIndex++;
				_currentIndex++;
				while (_currentIndex >= _currentNode.EntryCount)
				{
					_currentIndex = 0;
					_currentNode = _currentNode.Next;
					if (_currentNode == null)
						return false;
				}
				
				return true;
			}
			
			/// <summary>
			/// Moves the scan backward one element, returning true if the scan is positioned on an element within the list.
			/// </summary>
			/// <returns>True if the scan is positioned on an element within the list after the move, false otherwise.</returns>
			public bool Prior()
			{
				_absoluteIndex--;
				_currentIndex--;
				while (_currentIndex < 0)
				{
					_currentNode = _currentNode.Prior;
					if (_currentNode == null)
						return false;
					
					_currentIndex = _currentNode.EntryCount - 1;
				}
				
				return true;
			}
		}
		
		/// <summary>
		/// The currently cached scan for use in getting and setting elements in the partitioned list.
		/// </summary>
		private Scan _currentScan;

		/// <summary>
		/// Gets a scan positioned on the given index, using the currently cached scan if it is within the configured threshold distance.
		/// </summary>
		/// <param name="index">The position on which the scan should be opened.</param>
		/// <returns>A Scan instance positioned on the given index.</returns>
		private Scan GetScan(int index)
		{
			if (_currentScan != null)
			{
				if (Math.Abs(_currentScan.AbsoluteIndex - index) < PartitionedListParameters.UseCurrentScanThreshold && _currentScan.AbsoluteIndex < Count)
				{
					while (_currentScan.AbsoluteIndex < index)
						if (!_currentScan.Next())
							break;
							
					while (_currentScan.AbsoluteIndex > index)
						if (!_currentScan.Prior())
							break;
							
					if (_currentScan.AbsoluteIndex != index)
						_currentScan = null;
				}
				else
					_currentScan = null;
			}
			
			if (_currentScan == null)
				_currentScan = new Scan(GetPath(index));
				
			return _currentScan;
		}
		
		/// <summary>
		/// Clears the cached scan.
		/// </summary>
		private void ResetScan()
		{
			_currentScan = null;
		}

		/// <summary>
		/// Gets a range of elements as an array.
		/// </summary>
		/// <param name="index">The index of the first element to be retrieved.</param>
		/// <param name="count">The maximum number of elements to be retrieved.</param>
		public T[] Get(int index, int count)
		{
			Scan scan = GetScan(index);
			T[] results = new T[Math.Min(Count - Math.Min(Count, index), count)];
			for (int localIndex = 0; localIndex < results.Length; localIndex++)
			{
				results[localIndex] = scan.Current;
				if (!scan.Next())
					break;
			}
			return results;
		}
		
		/// <summary>
		/// Gets the element at the specified index.
		/// </summary>
		public T Get(int index)
		{
			return GetScan(index).Current;
		}
		
		/// <summary>
		/// Gets all the elements in the list.
		/// </summary>
		public T[] Get()
		{
			return Get(0, Count);
		}
		
		/// <summary>
		/// Sets the element at the given index to the given value.
		/// </summary>
		public void Set(int index, T tempValue)
		{
			// TODO: This would have to be modified in the presence of secondary indexes
			GetScan(index).Current = tempValue;
		}


		public void Move(int fromIndex, int toIndex)
		{
			Move(fromIndex, toIndex, 1);
		}


		public void Move(int fromIndex, int toIndex, int count)
		{
			if (toIndex != fromIndex)
			{
				T[] sourceRows = Get(fromIndex, count);

				for (int i = 0; i < sourceRows.Length; ++i)
					RemoveAt(fromIndex);

				for (int i = sourceRows.Length - 1; i >= 0; --i)
					Insert(toIndex, sourceRows[i]);
			}
		}
	
		/// <summary>
		/// Returns the path from the root to the leaf node containing the given index in the list.
		/// </summary>
		private PartitionedListPath GetPath(int index)
		{
			PartitionedListPath path = new PartitionedListPath(index);
			_root.GetPath(path, index);
			return path;
		}		

		private void InternalInsert(PartitionedListPath path, T element)
		{
			PartitionedListNode splitNode = null;
			for (int index = path.Length - 1; index >= 0; index--)
			{
				PartitionedListPathStep step = path[index];
				if (index == path.Length - 1)
				{
					PartitionedListDataNode<T> node = step.Node as PartitionedListDataNode<T>;
					if (node.EntryCount >= node.Capacity)
					{
						PartitionedListDataNode<T> newNode = new PartitionedListDataNode<T>(node.Capacity);

						// Thread the new node into the leaf list
						newNode.Prior = node;
						newNode.Next = node.Next;
						node.Next = newNode;
						if (newNode.Next != null)
							newNode.Next.Prior = newNode;
							
						// Reset the head and tail of the list, if necessary
						if (newNode.Prior == null)
							_head = newNode;
							
						if (newNode.Next == null)
							_tail = newNode;
							
						// Compute the pivot point
						int pivotIndex = node.EntryCount / 2;
						
						// Insert the upper half of the entries from the data node into the new node
						for (int elementIndex = pivotIndex; elementIndex < node.EntryCount; elementIndex++)
						{
							T _temp = node.Elements[elementIndex];
							newNode.Insert(_temp, elementIndex - pivotIndex);
							_reverseIndex[_temp] = newNode;
						}
						
						// Remove the upper half of the entries from the data node
						for (int elementIndex = node.EntryCount - 1; elementIndex >= pivotIndex; elementIndex--)
						{
							node.Delete(elementIndex);
						}
							
						// Insert the new entry into the appropriate node
						if (step.EntryIndex >= pivotIndex)
						{
							newNode.Insert(element, step.EntryIndex - pivotIndex);
							_reverseIndex.Add(element, newNode);
						}
						else
						{
							node.Insert(element, step.EntryIndex);
							_reverseIndex.Add(element, node);
						}
							
						// Set the split node to the new node
						splitNode = newNode;
					}
					else
					{
						// Insert the element at the correct index						
						node.Insert(element, step.EntryIndex);
						_reverseIndex.Add(element, node);
						
						// Clear the split node
						splitNode = null;
					}
				}
				else
				{
					PartitionedListRoutingNode node = step.Node as PartitionedListRoutingNode;
					if (splitNode != null)
					{
						int entryIndex = step.EntryIndex + 1; // Add 1 because the index on a routing step is always an existing element
						if (node.EntryCount >= node.Capacity)
						{
							PartitionedListRoutingNode newNode = new PartitionedListRoutingNode(node.Capacity);
							
							int pivotIndex = node.EntryCount / 2;
							int pivotKey = 0;
							
							// Insert the upper half of the entries from the routing node into the new node
							for (int elementIndex = pivotIndex; elementIndex < node.EntryCount; elementIndex++)
							{
								PartitionedListNode _temp = node.Nodes[elementIndex];								
								newNode.Insert(pivotKey, _temp, elementIndex - pivotIndex);
								_temp.Parent = newNode;
								pivotKey += _temp.Count;
							}
								
							// Remove the upper half of the entries from the routing node
							for (int elementIndex = node.EntryCount - 1; elementIndex >= pivotIndex; elementIndex--)
							{
								node.Delete(elementIndex);
							}
								
							// Insert the new entry into the appropriate node
							if (entryIndex >= pivotIndex)
							{
								
								newNode.Insert(0, splitNode, entryIndex - pivotIndex);
								splitNode.Parent = newNode;
								newNode.UpdateOffsets(0);
								node.UpdateOffsets(pivotIndex - 1);
							}
							else
							{								
								node.Insert(0, splitNode, entryIndex);
								splitNode.Parent = node;
								node.UpdateOffsets(entryIndex);
								newNode.UpdateOffsets(0);
							}
								
							// Set the split node to the new node
							splitNode = newNode;
						}
						else
						{
						
							node.Insert(0, splitNode, entryIndex);
							splitNode.Parent = node;
							node.UpdateOffsets(entryIndex);
							splitNode = null;
						}
					}
					else
					{
						node.UpdateOffsets(step.EntryIndex);
					}
				}
				
				if ((splitNode != null) && (index == 0))
				{
					// Grow the tree
					PartitionedListRoutingNode newNode = new PartitionedListRoutingNode(Fanout);					
					
					newNode.Insert(0, step.Node, 0);
					step.Node.Parent = newNode;
					newNode.Insert(step.Node.Count, splitNode, 1);
					splitNode.Parent = newNode;
					newNode.UpdateOffsets(0);
					_root = newNode;
					_height++;
				}
			}
		}
		
		/// <summary>
		/// Moves all rows from ASourceNode into ATargetNode.
		/// </summary>
		private void Merge(PartitionedListNode leftNode, PartitionedListNode rightNode)
		{
			if (rightNode is PartitionedListDataNode<T>)
			{
				PartitionedListDataNode<T> localLeftNode = leftNode as PartitionedListDataNode<T>;
				PartitionedListDataNode<T> localRightNode = rightNode as PartitionedListDataNode<T>;

				for (int index = 0; index < localRightNode.EntryCount; index++)
				{	
					T _temp = localRightNode.Elements[index];
					localLeftNode.Insert(_temp, localLeftNode.EntryCount);
					_reverseIndex[_temp] = localLeftNode;

				}

				for (int index = localRightNode.EntryCount - 1; index >= 0; index--)
				{
					localRightNode.Delete(index);
				}

				localLeftNode.Next = localRightNode.Next;
				if (localLeftNode.Next != null)
					localLeftNode.Next.Prior = localLeftNode;
				else
					_tail = localLeftNode;
			}
			else
			{
				PartitionedListRoutingNode localLeftNode = leftNode as PartitionedListRoutingNode;
				PartitionedListRoutingNode localRightNode = rightNode as PartitionedListRoutingNode;
				
				int updateFromIndex = localLeftNode.EntryCount;

				for (int index = 0; index < localRightNode.EntryCount; index++)
				{
					PartitionedListNode _temp = localRightNode.Nodes[index];
					localLeftNode.Insert(0, _temp, localLeftNode.EntryCount);
					_temp.Parent = localLeftNode;
				}

				for (int index = localRightNode.EntryCount - 1; index > 0; index--)
				{
					localRightNode.Delete(index);
				}
				
				localLeftNode.UpdateOffsets(updateFromIndex);
			}
		}
		
		private void InternalDelete(PartitionedListPath path)
		{
			// Delete the element from the leaf node, collapsing nodes up the tree as necessary
	
			for (int index = path.Length - 1; index >= 0; index--)
			{
				PartitionedListPathStep step = path[index];
				if (index == path.Length - 1)
				{
					PartitionedListDataNode<T> node = step.Node as PartitionedListDataNode<T>;
					T _temp = node.Elements[step.EntryIndex];
					node.Delete(step.EntryIndex);
					_reverseIndex.Remove(_temp);
				}
				else
				{
					PartitionedListRoutingNode node = step.Node as PartitionedListRoutingNode;
					
					if (step.EntryIndex > 0)
					{
						// Compare the affected node with the node to the left
						if ((node.Nodes[step.EntryIndex - 1].EntryCount + node.Nodes[step.EntryIndex].EntryCount) <= node.Nodes[step.EntryIndex].Capacity)
						{
							// Merge the right node into the left node and delete the entry
							Merge(node.Nodes[step.EntryIndex - 1], node.Nodes[step.EntryIndex]);
							node.Nodes[step.EntryIndex].Parent = null;
							node.Delete(step.EntryIndex);
						}
					}
					
					if (step.EntryIndex < node.EntryCount - 1)
					{
						// Compare the affected node with the node to the right
						if ((node.Nodes[step.EntryIndex].EntryCount + node.Nodes[step.EntryIndex + 1].EntryCount) <= node.Nodes[step.EntryIndex].Capacity)
						{
							// Merge the right node into the affected node and delete the entry
							Merge(node.Nodes[step.EntryIndex], node.Nodes[step.EntryIndex + 1]);
							node.Nodes[step.EntryIndex + 1].Parent = null;
							node.Delete(step.EntryIndex + 1);
						}
					}
					
					node.UpdateOffsets(step.EntryIndex > 0 ? step.EntryIndex - 1 : step.EntryIndex);
					
					// Delete the routing entry if the node it points to is empty
					int entryIndex = step.EntryIndex >= node.EntryCount ? step.EntryIndex - 1 : step.EntryIndex;
					PartitionedListNode _temp = node.Nodes[entryIndex];
					if (_temp.Count == 0)
					{
						_temp.Parent = null;
						node.Delete(entryIndex);
					}

					// Shrink the tree if there is only one routing entry in the root
					if ((index == 0) && (node.EntryCount == 1))
					{
						node.Nodes[0].Parent = null;
						_root = node.Nodes[0];
						_height--;
					}
				}
			}
		}

		#region IEnumerable Members

		/// <summary>
		/// Returns an IEnumerator for iterating over the elements in the list.
		/// </summary>
		IEnumerator IEnumerable.GetEnumerator()
		{
			return GetEnumerator();
		}

		#endregion

		#region IEnumerable<T> Members

		/// <summary>
		/// Returns an enumerator for iterating over the elements in the list.
		/// </summary>
		/// <returns></returns>
		public IEnumerator<T> GetEnumerator()
		{
			return new PartitionedListEnumerator(this);
		}
		
		/// <summary>
		/// Represents an enumerator for iterating over the elements in the list.
		/// </summary>
		public class PartitionedListEnumerator : IEnumerator<T>
		{
			#region IEnumerator<T> Members
			
			/// <summary>
			/// Constructs a new PartitionedListEnumerator for the given partitioned list.
			/// </summary>
			/// <param name="list"></param>
			internal PartitionedListEnumerator(PartitionedList<T> list)
			{
				_list = list;
				_scan = _list.GetScan(0);
				_atFirst = true;
			}
			
			private PartitionedList<T> _list;
			private Scan _scan;
			private bool _atFirst;

			/// <summary>
			/// Returns the current element in the enumerator.
			/// </summary>
			public T Current
			{
				get { return _atFirst ? default(T) : _scan.Current; }
			}

			#endregion

			#region IDisposable Members

			/// <summary>
			/// Disposes the enumerator
			/// </summary>
			public void Dispose()
			{
				_scan = null;
				_list = null;
			}

			#endregion

			#region IEnumerator Members

			/// <summary>
			/// Returns the current element in the enumerator
			/// </summary>
			object IEnumerator.Current
			{
				get { return Current; }
			}

			/// <summary>
			/// Moves to the next element in the list, returning false if there are no more elements.
			/// </summary>
			/// <returns>True if the enumerator is positioned on an element in the list after the move, false otherwise.</returns>
			public bool MoveNext()
			{
				if (_atFirst)
				{
					_atFirst = false;
					return true;
				}
				return _scan.Next();
			}

			/// <summary>
			/// Resets the enumerator to the beginning of the list.
			/// </summary>
			public void Reset()
			{
				_scan = _list.GetScan(0);
				_atFirst = true;
			}

			#endregion
		}

		#endregion

		#region ICollection<T> Members

		/// <summary>
		/// The number of elements in the list.
		/// </summary>
		public int Count { get { return _root.Count; } }
		
		public bool IsReadOnly
		{
			get { return false; }
		}

		/// <summary>
		/// Adds the given element to the end of the list.
		/// </summary>
		public void Add(T element)
		{
			Insert(Count, element);
		}
		
		/// <summary>
		/// Clears the elements in the list.
		/// </summary>
		/// <remarks>Note that this is equivalent to removing all the items in the list using repeated RemoveAt(Count - 1) invocations.</remarks>
		public void Clear()
		{
			while (Count > 0)
				RemoveAt(Count - 1);
		}
		
		/// <summary>
		/// Returns true if the list contains an element that is equal to the given element (using IComparable{T}), false otherwise.
		/// </summary>
		public bool Contains(T element)
		{
			return IndexOf(element) >= 0;
		}

		public void CopyTo(T[] array, int arrayIndex)
		{
			throw new NotImplementedException();
		}

		/// <summary>
		/// Removes the given element from the list if it is present.
		/// </summary>
		/// <returns>True if the element was removed from the list, false if the element was not present in the list.</returns>
		public bool Remove(T element)
		{
			int index = IndexOf(element);

			if (index >= 0)
			{
				RemoveAt(index);
				return true;
			}

			return false;
		}

		#endregion

		#region IList<T> Members

		/// <summary>
		/// Gets or sets the element at the given index.
		/// </summary>
		public T this[int index]
		{
			get { return Get(index); }
			set { Set(index, value); }
		}
		
		/// <summary>
		/// Returns the index of the given element in the list.
		/// </summary>
		/// <remarks>
		/// This method uses a linear search to find the element, using IComparable{T} (or reference comparison if IComparable{T} is not supported) to determine equality.
		/// </remarks>
		public int IndexOf(T element)
		{
			PartitionedListDataNode<T> node;
			if (_reverseIndex.TryGetValue(element, out node))
			{
				int index = FindIndex(node, element);
				PartitionedListNode current = node;
				while (current.Parent != null)
				{
					var parent = current.Parent as PartitionedListRoutingNode;
					int parentIndex = FindIndex(parent, current);
					index += parent.Keys[parentIndex];
					current = parent;
				}

				return index;
			}
			else
				return -1;
		}

		private int FindIndex(PartitionedListDataNode<T> parent, T child)
		{
			for (int i = 0; i < parent.Elements.Length; ++i)
			{
				if (parent.Elements[i].Equals(child))
					return i;
			}

			return -1;
		}

		private int FindIndex(PartitionedListRoutingNode parent, PartitionedListNode child)
		{
			for (int i = 0; i < parent.Nodes.Length; ++i)
			{
				if (parent.Nodes[i] == child)
					return i;
			}

			return -1;
		}

		/// <summary>
		/// Inserts the given element at the specified index in the list.
		/// </summary>
		public void Insert(int index, T element)
		{
			ResetScan();
			
			PartitionedListPath path = GetPath(index);
			InternalInsert(path, element);
		}
		
		/// <summary>
		/// Removes the element at the specified index in the list.
		/// </summary>
        public void RemoveAt(int index)
        {
            ResetScan();

            PartitionedListPath path = GetPath(index);
            if (!path.Found)
                throw new IndexOutOfRangeException(String.Format("Index ({0}) out of range.", index));

            InternalDelete(path);
        }
		
		#endregion

		#region IList Members

		int IList.Add(object tempValue)
		{
			Add((T)tempValue);
			return Count - 1;
		}

		void IList.Clear()
		{
			Clear();
		}

		bool IList.Contains(object tempValue)
		{
			return Contains((T)tempValue);
		}

		int IList.IndexOf(object tempValue)
		{
			return IndexOf((T)tempValue);
		}

		void IList.Insert(int index, object tempValue)
		{
			Insert(index, (T)tempValue);
		}

		bool IList.IsFixedSize
		{
			get { return false; }
		}

		bool IList.IsReadOnly
		{
			get { return IsReadOnly; }
		}

		void IList.Remove(object tempValue)
		{
			Remove((T)tempValue);
		}

		void IList.RemoveAt(int index)
		{
			RemoveAt(index);
		}

		object IList.this[int index]
		{
			get { return this[index]; }
			set { this[index] = (T)value; }
		}

		#endregion

		#region ICollection Members

		void ICollection.CopyTo(Array array, int index)
		{
			throw new NotImplementedException();
		}

		int ICollection.Count
		{
			get { return Count; }
		}

		bool ICollection.IsSynchronized
		{
			get { return false; }
		}

		object ICollection.SyncRoot
		{
			get { return this; }
		}

		#endregion
	}
	
	/// <summary>
	/// Represents a step on a path of element list nodes.
	/// </summary>
	internal class PartitionedListPathStep
	{
		/// <summary>
		/// Constructs a new path step.
		/// </summary>
		public PartitionedListPathStep(PartitionedListNode node, int entryIndex)
		{
			_node = node;
			_entryIndex = entryIndex;
		}
		
		private PartitionedListNode _node;
		/// <summary>
		/// The node at this step of the path
		/// </summary>
		public PartitionedListNode Node { get { return _node; } }
		
		private int _entryIndex;
		/// <summary>
		/// The index into the node at this step of the path
		/// </summary>
		public int EntryIndex { get { return _entryIndex; } }
	}
	
	/// <summary>
	/// Represents a path of nodes from the root to the leaf node containing a particular entry.
	/// </summary>
	/// <remarks>
	/// The 0th element in the path will always be the root node of the tree.
	/// The (n-1)th element will always be a data node (leaf).
	/// A valid path will always contain at least one node.
	/// </remarks>
	internal class PartitionedListPath
	{
		/// <summary>
		/// Constructs a new empty element list path for the given index
		/// </summary>
		public PartitionedListPath(int searchIndex)
		{
			_searchIndex = searchIndex;
		}

		public PartitionedListPath() : this(0) {}
	
		/// <summary>
		/// Internal list that stores the steps along the path
		/// </summary>
		private List<PartitionedListPathStep> _path = new List<PartitionedListPathStep>();
		
		private int _searchIndex;
		/// <summary>
		/// The index of the entry in the list to which the path leads.
		/// </summary>
		public int SearchIndex { get { return _searchIndex; } }
		
		private bool _found;
		/// <summary>
		/// Indicates whether the index contains an entry at SearchIndex
		/// </summary>
		public bool Found { get { return _found; } set { _found = value; } }
		
		/// <summary>
		/// Adds the given step to the path
		/// </summary>
		public void Add(PartitionedListPathStep node)
		{
			_path.Add(node);
		}

		public void Insert(PartitionedListPathStep node)
		{
			_path.Insert(0, node);
		}
		
		/// <summary>
		/// Gets the step at the given index
		/// </summary>
		public PartitionedListPathStep this[int index] { get { return _path[index]; } }
		
		/// <summary>
		/// The length of the path
		/// </summary>
		public int Length { get { return _path.Count; } }
	}
	
	/// <summary>
	/// Represents a node in the element list.
	/// </summary>
	internal abstract class PartitionedListNode
	{
		public PartitionedListNode Parent
		{
			get { return _parent; }
			set 
			{
				if (value != _parent)
					_parent = value;
			}			
		}
		
		private PartitionedListNode _parent;

		protected int _count = 0;
		/// <summary>
		/// The number of elements in the node and all sub-nodes, recursively
		/// </summary>
		public int Count { get { return _count; } }
		
		protected int _capacity;
		/// <summary>
		/// Specifies the maximum number of elements the node can store before it will be split
		/// </summary>
		public int Capacity { get { return _capacity; } }
		
		protected int _entryCount = 0;
		/// <summary>
		/// The absolute number of elements in the node
		/// </summary>
		public int EntryCount { get { return _entryCount; } } 
		
		/// <summary>
		/// Finds the path through this node to the given index.
		/// </summary>
		/// <remarks>
		/// Note that AIndex is relative to this node, not to the tree overall.
		/// </remarks>
		public abstract void GetPath(PartitionedListPath path, int index);
	}
	
	internal class PartitionedListDataNode<T> : PartitionedListNode
	{
		public PartitionedListDataNode(int capacity) : base()
		{
			_capacity = capacity;
			_elements = new T[capacity];
		}
		
		private T[] _elements;
		public T[] Elements { get { return _elements; } }

		public PartitionedListDataNode<T> Prior;
		public PartitionedListDataNode<T> Next;
		
		public override void GetPath(PartitionedListPath path, int index)
		{
			if (index > _entryCount)
				throw new IndexOutOfRangeException(String.Format("Index ({0}) out of range.", index));
				
			path.Add(new PartitionedListPathStep(this, index));
			path.Found = index < _entryCount;
		}

		public void Insert(T element, int entryIndex)
		{
			// Slide all entries above the insert index
			Array.Copy(_elements, entryIndex, _elements, entryIndex + 1, _entryCount - entryIndex);

			// Set the new entry data			
			_elements[entryIndex] = element;

			// Increment entry count			
			_entryCount++;
			
			// Increment absolute count
			_count++;
		}
		
		public void Delete(int entryIndex)
		{
			// Slide all entries above the insert index
			Array.Copy(_elements, entryIndex + 1, _elements, entryIndex, _entryCount - entryIndex - 1);
			
			// Decrement EntryCount
			_entryCount--;
			
			// Clear the deleted slot (otherwise G/C will not be able to reclaim the element)
			_elements[_entryCount] = default(T);
			
			// Decrement absolute count
			_count--;
		}
	}
	
	internal class PartitionedListRoutingNode : PartitionedListNode
	{
		public PartitionedListRoutingNode(int fanout)
		{
			_capacity = fanout;
			_keys = new int[fanout];
			_nodes = new PartitionedListNode[fanout];
		}
		
		private int[] _keys;
		public int[] Keys { get { return _keys; } }
		
		private PartitionedListNode[] _nodes;
		public PartitionedListNode[] Nodes { get { return _nodes; } }
		
		private bool Search(int index, out int entryIndex)
		{
			int lo = 0;
			int hi = _entryCount - 1;
			int localIndex = 0;
			int result = -1;
			
			while (lo <= hi)
			{
				localIndex = (lo + hi) / 2;
				result = _keys[localIndex] < index ? -1 : (_keys[localIndex] > index ? 1 : 0);
				if (result == 0)
					break;
				else if (result > 0)
					hi = localIndex - 1;
				else // if (LResult < 0) unnecessary
					lo = localIndex + 1;
			}
			
			if (result == 0)
				entryIndex = localIndex;
			else
				entryIndex = lo;
				
			return result == 0;
		}

		public override void GetPath(PartitionedListPath path, int index)
		{
			int entryIndex = 0;
			bool result = Search(index, out entryIndex);
			entryIndex = result ? entryIndex : (entryIndex - 1);
			path.Add(new PartitionedListPathStep(this, entryIndex));
			
			_nodes[entryIndex].GetPath(path, index - _keys[entryIndex]);
		}
		
		public void Insert(int key, PartitionedListNode node, int entryIndex)
		{
			// Slide all entries above the insert index
			Array.Copy(_keys, entryIndex, _keys, entryIndex + 1, _entryCount - entryIndex);
			Array.Copy(_nodes, entryIndex, _nodes, entryIndex + 1, _entryCount - entryIndex);

			// Set the new entry data			
			_keys[entryIndex] = key;
			_nodes[entryIndex] = node;

			// Increment entry count			
			_entryCount++;
		}
		
		public void Delete(int entryIndex)
		{
			// Slide all entries above the insert index
			Array.Copy(_keys, entryIndex + 1, _keys, entryIndex, _entryCount - entryIndex - 1);
			Array.Copy(_nodes, entryIndex + 1, _nodes, entryIndex, _entryCount - entryIndex - 1);
			
			// Decrement EntryCount
			_entryCount--;
			
			// Clear the deleted slots
			_keys[_entryCount] = -1;
			_nodes[_entryCount] = null; // Otherwise G/C will not be able to reclaim the node
		}
		
		public void UpdateOffsets(int entryIndex)
		{
			_count = entryIndex <= 0 ? 0 : (_keys[entryIndex - 1] + _nodes[entryIndex - 1].Count);
			for (int index = entryIndex; index < _entryCount; index++)
			{
				_keys[index] = _count;
				_count += _nodes[index].Count;
			}
		}
	}
}
