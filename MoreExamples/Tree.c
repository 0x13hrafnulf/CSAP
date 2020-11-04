#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
// Data structure to store a Binary Search Tree node
struct Node
{
	char *data;
	int count;
	struct Node *left, *right;
};

struct fileRef 
{
	char *name;
	struct fileRef* next;
};
 
// Function to create a new binary tree node having given key
struct Node* newNode(char *key)
{
	struct Node* node = malloc(sizeof(struct Node));
	node->data = malloc(strlen(key)+1);
	strcpy(node->data,key);
	node->count=1;
	node->left = node->right = NULL;
 
	return node;
}
 
// Function to perform inorder traversal of the tree
void inorder(struct Node* root)
{
	if (root == NULL) return;
 
	inorder(root->left);
	printf("%s:%d\n",root->data,root->count);
	inorder(root->right);
}

// Recursive function to insert an key into BST using a reference parameter
struct Node* insert(struct Node* root, char *key)
{
	// if the root is null, create a new node an return it
	if (root == NULL)
	{
		root = newNode(key);
		return root;
	}
 
	// if given key is less than the root node, recurse for left subtree
	// else recurse for right subtree
//	printf("V:%s\n",root->data);
	int cmp=strcmp(key,root->data);

	if (cmp<0)
		root->left=insert(root->left, key);
	else if(cmp>0)	// key >= root->data
		root->right=insert(root->right, key);
	else root->count++;
}

// main function
int main()
{
	struct Node* root = NULL;
	char* keys[] = { "15", "10", "20", "8", "12", "16", "25", "15", "10" };
 
	for (int key=0; key<sizeof(keys)/sizeof(keys[0]); key++) {
//		printf("Insert:%s\n",keys[key]);
		root=insert(root, keys[key]);
	}
 
	inorder(root);
 
	return 0;
}
