//---------------------------------------------------------------------
struct NODE { 
	struct NODE* left; 
	struct NODE* right; 
	double data; 
}; 

struct NODE {
	bool is_leaf;
	union {
		struct
		{
			struct NODE* left;
			struct NODE* right;
		} internal;
		double data;
	} info;
};
i
//-----------------------------------------------------------------------

typedef enum { INTEGER, STRING, REAL, POINTER } Type;

typedef struct
{
  Type type;
  union {
  int integer;
  char *string;
  float real;
  void *pointer;
  } x;
} Value;


Value As_integer(int v)
{
  Value v;
  v.type = INTEGER;
  v.x.integer = v;
  return v;
}
