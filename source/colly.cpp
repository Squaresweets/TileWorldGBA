#include "Colly.h"
#include <algorithm>

namespace cl
{
	Rect::Rect()
	{
		X = Y = Width = Height = 0;
	}
	Rect::Rect(float x, float y, float w, float h)
	{
		X = x;
		Y = y;
		Width = w;
		Height = h;
	}
	bool Rect::Intersects(const Rect& o, Rect& intersection)
	{
		// following code is from: https://github.com/SFML/SFML/blob/247b03172c34f25a808bcfdc49f390d619e7d5e0/include/SFML/Graphics/Rect.inl#L109

		// Compute the min and max of the first rectangle on both axes
		float minX1 = std::min(X, X + Width);
		float maxX1 = std::max(X, X + Width);
		float minY1 = std::min(Y, Y + Height);
		float maxY1 = std::max(Y, Y + Height);


		// Compute the min and max of the second rectangle on both axes
		float minX2 = std::min(o.X, o.X + o.Width);
		float maxX2 = std::max(o.X, o.X + o.Width);
		float minY2 = std::min(o.Y, o.Y + o.Height);
		float maxY2 = std::max(o.Y, o.Y + o.Height);


		// Compute the intersection boundaries
		float interLeft = std::max(minX1, minX2);
		float interTop = std::max(minY1, minY2);
		float interRight = std::min(maxX1, maxX2);
		float interBottom = std::min(maxY1, maxY2);


		// If the intersection is valid (positive non zero area), then there is an intersection
		if (interLeft < interRight && interTop < interBottom) {
			intersection = Rect(interLeft, interTop, interRight - interLeft, interBottom - interTop);
			return true;
		}

		intersection = Rect(0, 0, 0, 0);
		return false;
	}
    
	void GridWorld::Create(int width, int height, int cellW, int cellH)
	{
		m_cellH = cellH;
		m_cellW = cellW;
		m_w = width;
		m_h = height;

		// create the gird
		m_grid.resize(m_h, std::vector<int>(m_w, 0));

		// default filter - ID == 0 -> no collision else sold object
		GetCollisionType = [](int id) -> CollisionType
		{
			if (id == 0)
				return CollisionType::None;
			return CollisionType::Solid;
		};
	}
	Point GridWorld::Check(int steps, Rect bounds, Point goal, std::function<void(int, int, int, bool, GridWorld*)> func)
	{
		Rect intersect;

		// the increment per axis for each step
		float xInc = (goal.X - bounds.X) / steps;
		float yInc = (goal.Y - bounds.Y) / steps;

		// calculate subregion that needs to be checked
		Rect checkRegion(std::min(goal.X, bounds.X), std::min(goal.Y, bounds.Y), std::max(goal.X, bounds.X + bounds.Width), std::max(goal.Y, bounds.Y + bounds.Height));
		checkRegion.X = std::max(0, (int)(checkRegion.X - bounds.Width) / m_cellW);
		checkRegion.Y = std::max(0, (int)(checkRegion.Y - bounds.Height) / m_cellH);
		checkRegion.Width = std::min((int)(checkRegion.Width + bounds.Width) / m_cellW, m_w - 1);
		checkRegion.Height = std::min((int)(checkRegion.Height + bounds.Height) / m_cellH, m_h - 1);

		for (int i = 0; i < steps; i++) {
			// increment along x axis and check for collision
			bounds.X += xInc;
			for (int y = (int)checkRegion.Y; y <= (int)checkRegion.Height; y++) {
				for (int x = (int)checkRegion.X; x <= (int)checkRegion.Width; x++) {
					int id = m_grid[y][x];
					CollisionType type = GetCollisionType(id); // fetch the collision type through the filter GetCollisionType

					if (type == CollisionType::None) // no collision checking needed? just skip the body
						continue;

					Rect cell(x*m_cellW, y*m_cellH, m_cellW, m_cellH);

					if (cell.Intersects(bounds, intersect)) {
						// call the user function
						if (func != nullptr)
							func(id, x, y, true, this);

						// only check for collision if we encountered a solid object
						if (type != CollisionType::Solid)
							continue;

						float xInter = intersect.Width;
						float yInter = intersect.Height;

						if (xInter < yInter) {
							if (bounds.X < cell.X)
								xInter *= -1; // "bounce" in the direction that depends on the body and user position

							bounds.X += xInter; // move the user back

							break;
						}
					}
				}
			}

			// increment along y axis and check for collision - repeat everything for Y axis
			bounds.Y += yInc;
			for (int y = (int)checkRegion.Y; y <= (int)checkRegion.Height; y++) {
				for (int x = (int)checkRegion.X; x <= (int)checkRegion.Width; x++) {
					int id = m_grid[y][x];
					CollisionType type = GetCollisionType(id);

					if (type == CollisionType::None)
						continue;

					Rect cell(x*m_cellW, y*m_cellH, m_cellW, m_cellH);

					if (cell.Intersects(bounds, intersect)) {
						if (func != nullptr)
							func(id, x, y, false, this);

						if (type == CollisionType::Cross)
							continue;

						float xInter = intersect.Width;
						float yInter = intersect.Height;

						if (yInter < xInter) {
							if (bounds.Y < cell.Y)
								yInter *= -1;

							bounds.Y += yInter;

							break;
						}
					}
				}
			}

		}

		return { bounds.X, bounds.Y };
	}
}