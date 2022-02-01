
//{{BLOCK(TileMap)

//======================================================================
//
//	TileMap, 32x32@4, 
//	+ palette 16 entries, not compressed
//	+ 16 tiles not compressed
//	Total size: 32 + 512 = 544
//
//	Time-stamp: 2022-01-31, 21:43:00
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TILEMAP_H
#define GRIT_TILEMAP_H

#define TileMapTilesLen 512
extern const unsigned int TileMapTiles[128];

#define TileMapPalLen 32
extern const unsigned int TileMapPal[8];

#endif // GRIT_TILEMAP_H

//}}BLOCK(TileMap)