/*
	loader   
	mperron (2019)
*/
#ifndef QS_LOADER_H
#define QS_LOADER_H

enum asset_type {
	AVT_UNSET,
	AVT_INTEGER,
	AVT_FLOAT,
	AVT_STRING,
	AVT_SCENE,
	AVT_SCENE_OVERLAY,
	AVT_ARR,
	AVT_KV
};

union asset_value {
	int i;
	double f;
	char *str;
	struct asset_arr *arr;
	struct asset_kv *kv;
};

typedef struct asset_arr {
	enum asset_type type;
	union asset_value value;

	struct asset_arr *n;
} asset_arr;

typedef struct asset_kv {
	char *key;

	enum asset_type type;
	union asset_value value;

	struct asset_kv *l, *r;
} asset_kv;

extern asset_kv *kv_tree_add(asset_kv *kv_tree, asset_kv *kv);
extern asset_kv *kv_tree_find(asset_kv *kv_tree, char *key);

typedef struct asset {
	char *name;

	asset_kv *kv_tree;

	struct asset *l, *r;
} asset;

extern asset *asset_tree_add(asset *asset_tree, asset *asset);
extern asset *asset_tree_find(asset *asset_tree, char *name);

extern asset *asset_load();

#endif
