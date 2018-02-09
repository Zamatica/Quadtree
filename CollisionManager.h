

#include "Quadtree.h"


template <typename T>
GameObject * GetObject()
{
	return nullptr;
}


class CollisionManager
{
	QuadTree<9> m_tree = QuadTree<9>(coord(0, 0), coord(0, 0), 1);

public:
	void Update(float dt)
	{
		if (dt)
		{
			m_tree.Clear();

			size_t numObjectsInGame = 9;

			for (auto i = 0; i < numObjectsInGame; ++i)
			{
				GameObject * object = GetObject<int>();

				if (!object /* || Not being rendered || deleted object*/)
				{
					continue;
				}

				m_tree.AddObject(object, object->GetComponent<Transform>().GetPosition());
			}

		}
	}
};

