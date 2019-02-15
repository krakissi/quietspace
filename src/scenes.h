/*
	scenes
	mperron (2018)

	Graphic blobs for Quiet Space.
	
	If you add a new blob, make sure it's in the graphics_decode method.
*/
#include "base64.h"

/*
	Decode the base64 graphics data and return a newly allocated buffer which must be freed.
*/
char *load_graphics(char *graphics){
	return base64_dec(graphics, strlen(graphics));
}

char *scene_lift_doors = (
#include "assets_enc/scene_lift_doors"
);

char *scene_desk = (
#include "assets_enc/scene_desk"
);

char *scene_desk_overlay = (
#include "assets_enc/scene_desk_overlay"
);
