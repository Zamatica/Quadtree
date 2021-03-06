

#pragma once

#include "GameObjects/GameObject.h"
#include "GameObjects/Components/Components.h"

#define MAX_DEPTH 5

struct coord
{
	coord(float x, float y) : x(x), y(y) {}
	float x = 0;
	float y = 0;
};


template <typename T, size_t size>
struct Array
{
	size_t m_size;
	T m_array[size];
	
	const T & operator[](size_t index) const
	{
		return m_array[index];
	}

	T & operator[](size_t index)
	{
		return m_array[index];
	}
};


template <int max_objects = 9>
class QuadTree
{
	coord m_minPoint;
	coord m_maxPoint;
	coord m_center;

	Array<GameObject *, max_objects> m_objects; 
	Array<QuadTree<max_objects> *, 4> m_children;

	int m_depth;
public:
	QuadTree(coord & minPoint, coord & maxPoint, int depth)
	 : m_minPoint(minPoint), m_maxPoint(maxPoint), m_center((minPoint.x + maxPoint.x) / 2, (minPoint.y + maxPoint.y) / 2),
	    m_depth(depth)
	{
	}

	~QuadTree()
	{
		Clear();
	}

	void Clear()
	{
		if (m_children.m_size)
		{
			for (auto i = 0; i < m_children.m_size; ++i)
			{
				delete m_children[i];
				m_children[i] = nullptr;
			}
			
			m_children.m_size = 0;
		}
	}


	Array<GameObject *, max_objects> & GetObjectList()
	{
		return m_objects;
	}


	void AddChildern()
	{
		if (m_depth > MAX_DEPTH)
		{
			return;
		}
		++m_depth;

		// c0
		coord newMin(m_minPoint.x, m_center.y);
		coord newMax(m_center.x, m_maxPoint.y);

		m_children[0] = new QuadTree<max_objects>(newMin, newMax, m_depth);
		++m_children.m_size;


		// c1
		newMin.y = m_minPoint.y;
		
		m_children[1] = new QuadTree<max_objects>(m_minPoint, m_center, m_depth);
		++m_children.m_size;


		// c2
		newMin.x = m_center.x;
		newMin.y = m_minPoint.y;
		newMax.x = m_maxPoint.x;
		newMax.y = m_center.y;

		m_children[2] = new QuadTree<max_objects>(newMin, m_maxPoint, m_depth);
		++m_children.m_size;


		// c3
		m_children[3] = new QuadTree<max_objects>(m_center, m_maxPoint, m_depth);
		++m_children.m_size;


		for (auto i = 0; i < m_objects.m_size; ++i)
		{
			GameObject * object = m_objects[i];
			
			File(object, object->GetComponent<Transform>().GetPosition(), true);

			m_objects[i] = nullptr;
			--m_objects.m_size;
		}
	}


	void DestroyChildren()
	{
		int newNumObjects = CollectObjects(m_objects);

		Clear();

		m_children.m_size = 0;
		m_objects.m_size = newNumObjects;
	}


	int CollectObjects(Array<GameObject *, max_objects> & list)
	{
		auto newNum = 1;
		if (m_children.m_size)
		{
			for (auto i = 0; i < m_children.m_size; ++i)
			{
				newNum += m_children[i]->CollectObjects(list);
			}
		}
		else
		{
			for (auto i = 0; i < m_objects.m_size; ++i)
			{
				AddToEnd(m_objects[i], list, m_objects.m_size);
			}
		}
		return newNum;
	}


	void AddObject(GameObject * object, coord & pos)
	{
		if (m_children.m_size == 0 && m_depth < MAX_DEPTH && m_objects.m_size >= max_objects - 1)
		{
			AddChildern();
		}

		if (m_depth >= MAX_DEPTH)
			return;

		if (m_children.m_size)
		{
			File(object, pos, true);
		}
		else
		{
			AddToEnd(object);
		}
	}


	void RemoveObject(GameObject * object, coord & pos)
	{
		if (m_children.m_size && m_objects.m_size < 1)
		{
			DestroyChildren();
		}

		if (m_children.m_size)
		{
			File(object, pos, false);
		}
		else
		{
			int posInVect = FindIndexOfObject(object);
			if (posInVect >= 0)
			{
				m_objects[posInVect] = nullptr;

				for (auto i = posInVect + 1; i < m_objects.m_size; ++i)
				{
					m_objects[i - 1] = m_objects[i];
					m_objects[i] = nullptr;
				}
				--m_objects.m_size;
			}
		}
	}


	void MoveObject(GameObject * object, coord & oldPos)
	{
		RemoveObject(object, oldPos);
		AddObject(object, oldPos);
	}


	int FindIndexOfObject(GameObject * object)
	{
		for (auto i = 0; i < m_objects.m_size; ++i)
		{
			if (m_objects[i] == object)
				return i;
		}

		return -1;
	}


	void AddToEnd(GameObject * object)
	{
		m_objects[m_objects.m_size++] = object;
	}


	void File(GameObject * object, coord & pos, bool add)
	{
		coord objectSize = object->GetComponent<Collider2D>.GetHixBox();
		objectSize.x /= 2;
		objectSize.y /= 2;

		float radius = sqrtf(objectSize.x * objectSize.x + objectSize.y * objectSize.y);

		coord top(pos.x, pos.y + radius);
		coord bottom(pos.x, pos.y - radius);
		coord left(pos.x - radius, pos.y);
		coord right(pos.x + radius, pos.y);

		// Right
		if (left.x > m_center.x)
		{
			// Up
			if (bottom.y > m_center.y)
			{
				m_children[3]->AddObject(object, pos);
			}

			// Down
			else if (top.y < m_center.y)
			{
				m_children[2]->AddObject(object, pos);
			}

			// In Both Planes
			else
			{
				m_children[3]->AddObject(object, pos);
				m_children[2]->AddObject(object, pos);
			}
		}

		// Left
		else if (right.x < m_center.x)
		{
			// Up
			if (bottom.y > m_center.y)
			{
				m_children[0]->AddObject(object, pos);
			}

			// Down
			else if (top.y < m_center.y)
			{
				m_children[1]->AddObject(object, pos);
			}

			// In Both Planes
			else
			{
				m_children[0]->AddObject(object, pos);
				m_children[1]->AddObject(object, pos);
			}
		}

		// Edge Cases
		else
		{
			// On the y-axis
			if (right.x > m_center.x && left.x < m_center.x)
			{
				// Origin
				if (top.y > m_center.y && bottom.y < m_center.y)
				{
					m_children[0]->AddObject(object, pos);
					m_children[1]->AddObject(object, pos);
					m_children[2]->AddObject(object, pos);
					m_children[3]->AddObject(object, pos);
				}

				// Top Half
				else if (bottom.y > m_center.y)
				{
					m_children[0]->AddObject(object, pos);
					m_children[3]->AddObject(object, pos);
				}

				// Bottom Half
				else if (top.y < m_center.y)
				{
					m_children[1]->AddObject(object, pos);
					m_children[2]->AddObject(object, pos);
				}

				// Edge Cases
				else
				{
					m_children[0]->AddObject(object, pos);
					m_children[1]->AddObject(object, pos);
					m_children[2]->AddObject(object, pos);
					m_children[3]->AddObject(object, pos);
				}
			}

			// On the x-axis
			else if (bottom.y < m_center.y && top.y > m_center.y)
			{
				// Left Half
				if (right.x < m_center.x)
				{
					m_children[0]->AddObject(object, pos);
					m_children[1]->AddObject(object, pos);
				}

				// Bottom Half
				else if (left.x > m_center.x)
				{
					m_children[2]->AddObject(object, pos);
					m_children[3]->AddObject(object, pos);
				}

				// Edge Cases
				else
				{
					m_children[0]->AddObject(object, pos);
					m_children[1]->AddObject(object, pos);
					m_children[2]->AddObject(object, pos);
					m_children[3]->AddObject(object, pos);
				}
			}
		}

	}
};






