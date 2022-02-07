#ifndef __COLLY_H__
#define __COLLY_H__

#include <vector>
#include <functional>

namespace cl
{
	/*
		2D point structure
	*/
	struct Point
	{
		float X, Y;
	};



	/*
		Rectangle structure. A rectangle has a position(top-left corner)
		and Width and Height property.
	*/
	class Rect
	{
	public:
		float X, Y, Width, Height;

		Rect();
		Rect(float x, float y, float width, float height);

		// does this rectangle intersect another rectangle?
		bool Intersects(const Rect& other, Rect& intersection);

		// is this rectangle exactly the same as another rectangle (has same position and size)
		inline bool operator==(const Rect& r) { return X == r.X && Y == r.Y && Width == r.Width && Height == r.Height; }
	};



	/*
		Collision types - can be collided, is a 'pass-through' object or
		should be ignored. Later we can add other types of collision
		handling (example: one way blocks)
	*/
	enum class CollisionType
	{
		None,	// no collision will occur
		Solid,	// collision will be handled
		Cross	// used for coins and other pick ups
	};



	/*
		Body is just a rectangle with addition information. It can have its own ID or 
		some user data attached. It also has a collision type.
	*/
	struct Body
	{
		int Id;
		Rect Bounds;
		CollisionType Type;
		void* UserData;

		bool operator==(const Body& bdy) { return Id == bdy.Id && Bounds == bdy.Bounds && Type == bdy.Type && UserData == bdy.UserData; }
	};


	/*
		GridWorld is a class similar to World and does almost everything similar to the
		World class except it is used for worlds where elements are organized in a grid.
		It should be used in tile worlds. GridWorld doesnt use QuadTree and it doesnt
		use cl::Body class. It only needs a 2D array of tile IDs and a function GetCollisionType
		which tells it whether a tile is CollisionType::Solid, CollisionType::Cross, etc...
	*/
	class GridWorld
	{
	public:
		// create a grid world with given width and height and cell size
		void Create(int width, int height, int cellW, int cellH);

		// set/get object on a given position
		inline void SetObject(int x, int y, int id) { m_grid[y][x] = id; }
		inline int GetObject(int x, int y) { return m_grid[y][x]; }

		// get world size
		inline int GetWidth() { return m_w; }
		inline int GetHeight() { return m_h; }

		// Check for collision between player and the grid world.
		// NOTE: read World::Check() comment to read about the "steps" parameter
		Point Check(int steps, Rect body, Point goal, std::function<void(int, int, int, bool, GridWorld*)> func = nullptr);

		// a "filter" which tells us certain CollisionType for each tile ID
		std::function<CollisionType(int)> GetCollisionType; 

	private:
		int m_w, m_h, m_cellW, m_cellH;
		std::vector<std::vector<int>> m_grid;
	};
}

#endif