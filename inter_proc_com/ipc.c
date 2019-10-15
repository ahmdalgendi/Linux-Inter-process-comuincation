#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>
#include<linux/sched.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include<linux/mutex.h>
#include<linux/spinlock.h>
#include<linux/string.h>
//
// Class to implement node
typedef struct ACLNode
{

	// MAIL Box Id
	unsigned int mx_id;

	// Array to hold pointers to node of different level
	struct ACLNode **forward;

}ACLNode;

ACLNode * init_ACLNode(unsigned int key, int level)
{
	//use kmalloc

	ACLNode * ret = (ACLNode*)kmalloc(sizeof(ACLNode),GFP_KERNEL);

	ret->mx_id = key;
	// Allocate memory to forward
	//use kmalloc

	ret->forward = (ACLNode**)kmalloc(sizeof(ACLNode*) * (level + 1),GFP_KERNEL);

	// Fill forward array with 0(NULL)
	memset(ret->forward, 0, sizeof(ACLNode*)*(level + 1));
	return  ret;
}

// Class for Skip list
typedef struct ACLSkipList
{
	// Maximum level for this skip list
	int MAXLVL;

	// P is the fraction of the nodes with level
	// i pointers also having level i+1 pointers
	int P;

	// current_node level of skip list
	int level;
	// size of the list
	int size;
	// pointer to header node
	ACLNode *header;


}ACLSkipList;

ACLSkipList init_ACLSkipList(int MAXLVL, int P)
{

	ACLSkipList ret;
	ret.MAXLVL = MAXLVL;
	ret.P = P;
	ret.level = 0;

	// create header node and initialize mx_id to -1
	ret.header = init_ACLNode(-1, MAXLVL);
	return  ret;
}

static unsigned int next_random = 9001;

static unsigned int generate_random_int(void) {
	next_random = next_random * 1103515245 + 12345;
	return (next_random / 65536) % 32768;
}

static void seed_random(unsigned int seed) {
	next_random = seed;
}

// create random level for node
int randomLevel_ACL(ACLSkipList *skipList)
{
	int lvl;
	int r;
	lvl = 0;
	r = generate_random_int();
	
	seed_random(1586325);

	while (r < 32768 / skipList->P && lvl < skipList->MAXLVL)
	{
		lvl++;
		r = generate_random_int();
	}

	return lvl;
}

// create new node
ACLNode* createNode(unsigned int key, int level)
{
	ACLNode *n = init_ACLNode(key, level);
	return n;
}

// Insert given mx_id in skip list
int insertElement_ACL(ACLSkipList* skipList, unsigned int key)
{
	ACLNode* n ;
	 int idx ;
	int i;
	ACLNode **update ;

	ACLNode *current_node ;
	current_node = skipList->header;

	// create update array and initialize it
	 idx = (skipList->MAXLVL) + 1;

	update = (ACLNode**) kmalloc(sizeof(ACLNode*) * idx  , GFP_KERNEL);
	memset(update, 0, sizeof(ACLNode*)*(skipList->MAXLVL + 1));

	/* start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] != NULL &&
			current_node->forward[i]->mx_id < key)
			current_node = current_node->forward[i];
		update[i] = current_node;
	}

	/* reached level 0 and forward pointer to
	right, which is desired position to
	insert mx_id.
	*/
	current_node = current_node->forward[0];

	/* if current_node is NULL that means we have reached
	to end of the level or current_node's mx_id is not equal
	to mx_id to insert that means we have to insert
	node between update[0] and current_node node */
	if (current_node == NULL || current_node->mx_id != key)
	{
		// Generate a random level for node
		int rlevel = randomLevel_ACL(skipList);

		// If random level is greater than list's current_node
		// level (node with highest level inserted in
		// list so far), initialize update value with pointer
		// to header for further use
		if (rlevel > skipList->level)
		{
			for ( i = skipList->level + 1;i < rlevel + 1;i++)
				update[i] = skipList->header;

			// Update the list current_node level
			skipList->level = rlevel;
		}

		// create new node with random level generated
		n = createNode(key, rlevel);

		// insert node by rearranging pointers
		for ( i = 0;i <= rlevel;i++)
		{
			n->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = n;
		}
		return 1; // insertion completed
	}
	return 0; // insertion failed
}

// Display skip list level wise
void displayList_ACL(ACLSkipList * skipList)
{
	int i;

	printk("\n*****Skip List*****\n");
	for ( i = 0;i <= skipList->level;i++)
	{
		ACLNode *node = skipList->header->forward[i];
		printk("Level %d: ", i);

		while (node != NULL)
		{
			printk("%d ", node->mx_id);
			node = node->forward[i];
		}
		printk("\n");
	}


}

// Display skip list level wise
void destroy_ACL(ACLSkipList *skipList)
{

	int len ,i,a;
	ACLNode ** todel ;
		ACLNode *node ;

	len = 0;
	for ( i = 0;i <= skipList->level;i++)
	{
		ACLNode *node = skipList->header->forward[i];


		while (node != NULL)
		{
			printk("%d ", node->mx_id);
			node = node->forward[i];
			len++;
		}
	}

	todel = (ACLNode**)kmalloc(sizeof(ACLNode*) * len,GFP_KERNEL);
	a = 0;
	for ( i = 0;i <= skipList->level;i++)
	{
		node = skipList->header->forward[i];
		while (node != NULL)
		{
			todel[a] = node;
			node = node->forward[i];

			a++;
		}
	}

	for ( i = 0;i < len;i++)
	{
		kfree(todel[i]);
	}
}

// Delete element from skip list
int deleteElement_ACL(ACLSkipList * skipList, unsigned int key)
{
	int i ;

	ACLNode *current_node ;

	// create update array and initialize it
	ACLNode **update ;
	current_node = skipList->header;

	// create update array and initialize it
	update = (ACLNode**) kmalloc(sizeof(ACLNode*) *  (skipList->MAXLVL + 1) , GFP_KERNEL);
	memset(update, 0, sizeof(ACLNode*)*(skipList->MAXLVL + 1));

	/*    start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] != NULL &&
			current_node->forward[i]->mx_id < key)
			current_node = current_node->forward[i];
		update[i] = current_node;
	}

	/* reached level 0 and forward pointer to
	   right, which is possibly our desired node.*/
	current_node = current_node->forward[0];

	// If current_node node is target node
	if (current_node != NULL && current_node->mx_id == key)
	{
		/* start from lowest level and rearrange
		   pointers just like we do in singly linked list
		   to remove target node */
		for ( i = 0;i <= skipList->level;i++)
		{
			/* If at level i, next node is not target
			   node, break the loop, no need to move
			  further level */
			if (update[i]->forward[i] != current_node)
				break;

			update[i]->forward[i] = current_node->forward[i];
		}

		// Remove levels having no elements
		while (skipList->level > 0 &&
			skipList->header->forward[skipList->level] == 0)
			(skipList->level)--;
		return 1; // found and deleted
	}
	return 0; // not found, so nothing deleted
}

// Search for element in skip list
int searchElement_ACL(ACLSkipList * skipList, unsigned int key)
{
	ACLNode *current_node ;
	int i;
	current_node = skipList->header;

	/*    start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] &&
			current_node->forward[i]->mx_id < key)
			current_node = current_node->forward[i];

	}

	/* reached level 0 and advance pointer to
	   right, which is possibly our desired node*/
	current_node = current_node->forward[0];

	// If current_node node have mx_id equal to
	// search mx_id, we have found our target node
	if (current_node && current_node->mx_id == key)
		return 1;  //found
	return 0; // notfound
}



// MailBox using liked lists to save the messages in each mail-bo
struct MLNode {
	unsigned char * msg;
	long  msg_len;
	struct MLNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct MailBox {
	struct MLNode *front, *rear;
	int size;
	ACLSkipList acl;
	long long id;
};

// A utility function to create a new linked list node.
struct MLNode* newNode(const unsigned char* msg, long len)
{    //use kmalloc

	struct MLNode* temp ;
	int i;
	temp = (struct MLNode*)kmalloc(sizeof(struct MLNode),GFP_KERNEL);
	//use kmalloc
	temp->msg = (unsigned char *)kmalloc(sizeof(char) * len,GFP_KERNEL);
	for ( i = 0; i < len; ++i) {
		temp->msg[i] = msg[i];
	}
	temp->msg_len = len;
	temp->next = NULL;
	return temp;
}

// A utility function to create an empty queue
struct MailBox *createMailBox(int mxLevel, int p, int ID)
{
	struct MailBox *q ;
	q = (struct MailBox *) kmalloc(sizeof(struct MailBox),GFP_KERNEL) ;
	q->front = q->rear = NULL;
	q->size = 0;
	q->acl = init_ACLSkipList(mxLevel, p);
	q->id = ID;
	return q;
}

// The function to add a key k to q
void pushMailBox(struct MailBox *q, const unsigned char *msg, long len)
{
	//     Create a new LL node
	struct MLNode* temp ;
	temp = newNode(msg, len);
	(q->size)++;
	// If queue is empty, then new node is front and rear both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
}

// Function to remove a key from given queue q
struct MLNode* popMailBox(struct MailBox *q)
{
	struct MLNode* temp;
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return NULL;
	(q->size)--;
	// Store previous front and move front one node ahead
	temp = q->front;
	

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;
	return temp;
}

int size_ML(struct MailBox* q)
{
	return q->size;
}

void destroyMailBox(struct MailBox *q)
{
	struct MLNode * node  ,*tmp;
	node = q->front;
	while (node != NULL)
	{
		tmp = node;
		node = node->next;
		kfree(tmp->msg);
		kfree(tmp);
	}
}






// Class to implement node
typedef struct MailBoxSkipListNode
{

	// MAIL Box Id
	struct MailBox  *mailBox;

	// Array to hold pointers to node of different level
	struct MailBoxSkipListNode **forward;

}MailBoxSkipListNode;

MailBoxSkipListNode * init_MailBoxSkipListNode(struct MailBox *mailBox, int level)
{
	//use kmalloc

	MailBoxSkipListNode * ret;
	ret = (MailBoxSkipListNode*)kmalloc(sizeof(MailBoxSkipListNode),GFP_KERNEL);

	ret->mailBox = mailBox;
	// Allocate memory to forward
	//use kmalloc

	ret->forward = (MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) * (level + 1),GFP_KERNEL);

	// Fill forward array with 0(NULL)
	memset(ret->forward, 0, sizeof(MailBoxSkipListNode*)*(level + 1));
	return  ret;
}

// Class for Skip list
typedef struct MailBoxSkipList
{
	// Maximum level for this skip list
	int MAXLVL;

	// P is the fraction of the nodes with level
	// i pointers also having level i+1 pointers
	int P;

	// current_node level of skip list
	int level;
	// size of the list
	int size;
	// pointer to header node
	MailBoxSkipListNode *header;


}MailBoxSkipList;

MailBoxSkipList *init_MailBoxSkipList(int MAXLVL, int P)
{
	int probs[5] , found,i ;
	MailBoxSkipList *ret ;
	struct MailBox *mailBox ;
	probs[1] = 2;
	probs[1] = 4;
	probs[1] = 8;
	probs[1] = 16;
	found = 0;
	for ( i = 0; i < 4; ++i) {
		if (P == probs[i])
			found++;
	}
	if (!found || MAXLVL < 1)
		return NULL;

	ret = (MailBoxSkipList*)kmalloc(sizeof(MailBoxSkipList),GFP_KERNEL);
	ret->MAXLVL = MAXLVL;
	ret->P = P;
	ret->level = 0;
	ret->size = 0;
	mailBox = createMailBox(MAXLVL, P, LONG_MIN);
	ret->header = init_MailBoxSkipListNode(mailBox, MAXLVL);
	return  ret;
}



// create random level for node
int randomLevel_MailBox(MailBoxSkipList *skipList)
{
	int r ;
	int lvl ;
	 r = generate_random_int();
	 lvl = 0;
	while (r < 32768 / skipList->P && lvl < skipList->MAXLVL)
	{
		lvl++;
		r = generate_random_int();
	}

	return lvl;
}

// create new node
MailBoxSkipListNode* createNode_MailBoxNode(struct MailBox * mailBox, int level)
{
	MailBoxSkipListNode *n ;
	n = init_MailBoxSkipListNode(mailBox, level);
	return n;
}

// Insert given mx_id in skip list
int insertElement_MailBoxSkipList(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *current_node ;
	MailBoxSkipListNode **update;
	struct MailBox *mailBox;
	int i ;
	int rlevel ;

	MailBoxSkipListNode* n ;

	current_node = skipList->header;

	// create update array and initialize it
	 update = 
		(MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*)* (skipList->MAXLVL + 1),GFP_KERNEL );
	memset(update, 0, sizeof(MailBoxSkipListNode*)*(skipList->MAXLVL + 1));

	mailBox = createMailBox(skipList->level, skipList->P, key);
	/* start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] != NULL &&
			current_node->forward[i]->mailBox->id < key)
			current_node = current_node->forward[i];
		update[i] = current_node;
	}

	/* reached level 0 and forward pointer to
	right, which is desired position to
	insert mx_id.
	*/
	current_node = current_node->forward[0];

	/* if current_node is NULL that means we have reached
	to end of the level or current_node's mx_id is not equal
	to mx_id to insert that means we have to insert
	node between update[0] and current_node node */
	if (current_node == NULL || current_node->mailBox->id != key)
	{
		// Generate a random level for node
		 rlevel = randomLevel_MailBox(skipList);

		// If random level is greater than list's current_node
		// level (node with highest level inserted in
		// list so far), initialize update value with pointer
		// to header for further use
		if (rlevel > skipList->level)
		{
			for ( i = skipList->level + 1;i < rlevel + 1;i++)
				update[i] = skipList->header;

			// Update the list current_node level
			skipList->level = rlevel;
		}

		// create new node with random level generated
		 n = createNode_MailBoxNode(mailBox, rlevel);

		// insert node by rearranging pointers
		for ( i = 0;i <= rlevel;i++)
		{
			n->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = n;
		}
		return 1; // insertion completed
	}
	return 0; // insertion failed
}

// Display skip list level wise
void displayList_MailBoxSkipList(MailBoxSkipList *skip_list)
{
	MailBoxSkipListNode *node ;
	int i;
	printk("\n*****Skip List*****\n");

	for ( i = 0;i <= skip_list->level;i++)
	{
		node = skip_list->header->forward[i];
		printk("Level %d: ", i);

		while (node != NULL)
		{
			printk("%lld ", node->mailBox->id);
			node = node->forward[i];
		}
		printk("\n");
	}


}

// Display skip list level wise
void destroy_MailBoxSkipList(MailBoxSkipList *skipList)
{

	int len ,i,a;
	MailBoxSkipListNode *node ;
	MailBoxSkipListNode ** todel;
	len = 0;
	for ( i = 0;i <= skipList->level;i++)
	{
		node = skipList->header->forward[i];


		while (node != NULL)
		{
			printk("%lld ", node->mailBox->id);
			node = node->forward[i];
			len++;
		}
	}

	todel = (MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) * len,GFP_KERNEL);
	a = 0;
	for ( i = 0;i <= skipList->level;i++)
	{
		node = skipList->header->forward[i];
		while (node != NULL)
		{
			todel[a] = node;
			node = node->forward[i];

			a++;
		}
	}

	for ( i = 0;i < len;i++)
	{
		destroyMailBox(todel[i]->mailBox);
		kfree(todel[i]);
	}
}

// Delete element from skip list
int deleteElement_MailBoxSkipList(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *current_node;

	// create update array and initialize it

	MailBoxSkipListNode **update;
	int i ;
	 current_node = skipList->header;

	// create update array and initialize it

	 update = 
		(MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) *   (skipList->MAXLVL + 1),GFP_KERNEL);
	memset(update, 0, sizeof(MailBoxSkipListNode*)*(skipList->MAXLVL + 1));



	/*    start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] != NULL &&
			current_node->forward[i]->mailBox->id < key)
			current_node = current_node->forward[i];
		update[i] = current_node;
	}

	/* reached level 0 and forward pointer to
	   right, which is possibly our desired node.*/
	current_node = current_node->forward[0];

	// If current_node node is target node
	if (current_node != NULL && current_node->mailBox->id == key)
	{
		/* start from lowest level and rearrange
		   pointers just like we do in singly linked list
		   to remove target node */
		for ( i = 0;i <= skipList->level;i++)
		{
			/* If at level i, next node is not target
			   node, break the loop, no need to move
			  further level */
			if (update[i]->forward[i] != current_node)
				break;

			update[i]->forward[i] = current_node->forward[i];
		}

		// Remove levels having no elements
		while (skipList->level > 0 &&
			skipList->header->forward[skipList->level] == 0)
			(skipList->level)--;
		return 1; // found and deleted
	}
	return 0; // not found, so nothing deleted
}

// Search for element in skip list
int searchElement_MailBoxSkipList(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *current_node ;
	int i;
	current_node = skipList->header;

	/*    start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] &&
			current_node->forward[i]->mailBox->id < key)
			current_node = current_node->forward[i];

	}

	/* reached level 0 and advance pointer to
	   right, which is possibly our desired node*/
	current_node = current_node->forward[0];

	// If current_node node have mx_id equal to
	// search mx_id, we have found our target node
	if (current_node && current_node->mailBox->id == key)
		return 1;  //found
	return 0; // notfound
}


struct MailBox * find_MailBox(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *current_node ;
	int i;
	current_node = skipList->header;

	/*    start from highest level of skip list
		move the current_node pointer forward while mx_id
		is greater than mx_id of node next to current_node
		Otherwise inserted current_node in update and
		move one level down and continue search
	*/
	for ( i = skipList->level; i >= 0; i--)
	{
		while (current_node->forward[i] &&
			current_node->forward[i]->mailBox->id < key)
			current_node = current_node->forward[i];

	}

	/* reached level 0 and advance pointer to
	   right, which is possibly our desired node*/
	current_node = current_node->forward[0];

	// If current_node node have mx_id equal to
	// search mx_id, we have found our target node
	if (current_node && current_node->mailBox->id == key)
		return current_node->mailBox;  //found
	return NULL; // notfound

}

int get_msgCount(MailBoxSkipList * skip_list , unsigned int key)
{
	struct MailBox * mail;
	mail = find_MailBox(skip_list, key);

	if (mail == NULL)
		return -1; // invalid key
	return mail->size;
}

//TODO
long mbx421_send_helper(MailBoxSkipList* skip_list ,unsigned int id, const unsigned char 
	*msg, long len)
{
	struct MailBox * mail ;

	mail = find_MailBox(skip_list, id);
	if (mail == NULL)
	{
		return 0; // failed
	}
	pushMailBox(mail, msg, len);
	return  1;
}
//TODO

long mbx421_recv_helper(MailBoxSkipList * skip_list,unsigned int id, unsigned char  *msg, long
	len)
{
	struct MailBox *ret;
	struct MLNode * poped ;
	
	int cpd ,i;

	ret = find_MailBox(skip_list ,id);
	poped = popMailBox(ret);
	len = min(len, poped->msg_len);
	cpd = __copy_to_user(msg , poped->msg, len);
	return cpd;
}


MailBoxSkipList *container;

/*long mbx421_init(unsigned int ptrs, unsigned int prob): 
Initializes the mailbox system, setting up the initial state of the skip list. 
The ptrs parameter specifies the maximum number of pointers any node in the list 
will be allowed to have. The prob parameter specifies the inverse of the probability 
that a node will be promoted to having an additional pointer in it (that is to say that 
if the function is called with prob = 2, then the probability that the node
will have 2 pointers is 1 / 2 and the probability that it will have 3 pointers is 1 / 4, and so on).
The only valid values for the prob parameter are 2, 4, 8, and 16 — 
any other value shall result in an error being returned. Additionally, 
the ptrs parameter must be non-zero — a zero value should result in an error being returned.

Returns 0 on success. Only the root user (the user with a uid of 0) shall be allowed to call this function.
*/
SYSCALL_DEFINE2(mbx421_init,unsigned int ,ptrs, unsigned int, prob ){
	printk("mbx421_init\n");

	if(get_current_cred()->uid.val != 0)
        return -EACCES;
  
	container = init_MailBoxSkipList(ptrs,prob);
	if(container == NULL)
		return -EINVAL;
	return 0;

}


/*long mbx421_shutdown(void): 
Shuts down the mailbox system, 
deleting all existing mailboxes and any messages contained therein. 
Returns 0 on success. Only the root user shall be allowed to call this function.
*/
SYSCALL_DEFINE0(mbx421_shutdown)
{
	printk("mbx421_shutdown\n");
	if(get_current_cred()->uid.val != 0)
        return -EACCES;
    if (container == NULL)
    {
    	return -EPERM;
    }
	destroy_MailBoxSkipList(container);

	return 0;
}

/*long mbx421_create(unsigned int id): 
Creates a new mailbox with the given id if it does not already exist 
(no duplicates are allowed). Returns 0 on success or an appropriate error on failure.
If an id of 0 or (264 - 1) is passed, 
this is considered an invalid ID and an appropriate error shall be returned. 
Only the root user shall be allowed to call this function.
*/
SYSCALL_DEFINE1(mbx421_create, unsigned int , id)
{
	printk("mbx421_create\n");
	if(get_current_cred()->uid.val != 0)
        return -EACCES;
	int ret = insertElement_MailBoxSkipList(container, id);
	if (ret)
	{
		return 0;		
	}
	return -EEXIST;
}

/*long mbx421_destroy(unsigned int id): 
Deletes the mailbox identified by id if it exists and the user has permission to do so. 
If the mailbox has any messages stored in it, these messages should be deleted. 
Returns 0 on success or an appropriate error code on failure. 
Only the root user shall be allowed to call this function.
*/

SYSCALL_DEFINE1(mbx421_destroy, unsigned int , id){
	printk("mbx421_destroy\n");
	if(get_current_cred()->uid.val != 0)
        return -EACCES;
	int ret = deleteElement_MailBoxSkipList(container, id);
	if(ret)
		return 0 ;
	return -ENOENT;
}

/*long mbx421_count(unsigned int id): 
Returns the number of messages in the mailbox identified by id if it
exists and the user has permission to access it. 
Returns an appropriate error code on failure.
*/

SYSCALL_DEFINE1(mbx421_count, unsigned int , id){
	printk("mbx421_count\n");
	int ret = get_msgCount(container, id);
	if(ret >= 0 )
		return ret ;
	return -ENOENT;

}

/*long mbx421_send(unsigned int id, const unsigned char __user *msg, long len): 
Sends a new message to the mailbox identified by id if it exists and the user has access to it. 
The message shall be read from the user-space pointer msg and shall be len bytes long. 
Returns 0 on success or an appropriate error code on failure.
*/
SYSCALL_DEFINE3(mbx421_send, unsigned int , id, const unsigned char __user, *msg, long ,len){
	unsigned char * kmesg;
	int ret , num;
	printk("mbx421_send\n");
	if(msg == NULL !! len <= 0)
		return -EINVAL;
	
	kmesg = ( unsigned char *)kmalloc(sizeof(char) * len);
	
	num = __copy_from_user(kmesg , mesg , len * sizeof(char));

	 ret = mbx421_send_helper(container, id, kmsg, len);
	if (ret)
	{
		return num ;
	}
	return -ENOENT;

}


/*long mbx421_recv(unsigned int id, unsigned char __user *msg, long len):
Reads the first message that is in the mailbox identified by id 
if it exists and the user has access to it, 
storing either the entire length of the message or len bytes to the user-space pointer msg, 
whichever is less. The entire message is then removed from the mailbox 
(even if len was less than the total length of the message). 
Returns the number of bytes copied to the user space pointer on success or an appropriate error code on 
failure.
*/

SYSCALL_DEFINE3(mbx421_recv, unsigned int , id,  unsigned char __user, *msg, long ,len){
	printk("mbx421_recv\n");

	if(msg == NULL !! len <= 0)
		return -EINVAL;
	

	int ret = mbx421_recv_helper(container, id, msg, len);
	if (ret)
	{
		return ret;
	}
	return -ENOENT;

}


/* long mbx421_length(unsigned int id): 
Retrieves the length (in bytes) of the first message pending in the mailbox identified by id, 
if it exists and the user has access to it. Returns the number of bytes in the first 
pending message in the mailbox on success, or an appropriate error code on failure.
*/

SYSCALL_DEFINE1(mbx421_length, unsigned int , id){
	int ret;
	printk("mbx421_length\n");
	ret = get_msgCount(container,id);
	if(ret != -1)
		return ret;

	return -ENOENT ;
}


/*long mbx421_acl_add(unsigned int id, pid_t process_id): 
Adds PID specified by process_id to the ACL for the mailbox with the ID of id if it
exists and the user has access to do so. Returns 0 on success or an appropriate error code on failure.
Only the root user shall be allowed to call this function.
*/


SYSCALL_DEFINE2(mbx421_acl_add, unsigned int , id,pid_t ,process_id){
	ACLSkipList * acl;
	int ret;
	struct MailBox * mailbox;
	printk("mbx421_acl_add\n");
	mailbox = find_MailBox(container, id);
	if(mailbox == NULL)
		return -ENOENT ;
	ret = insertElement_ACL(mailbox->acl, process_id);
	if (ret)
	{
		return 0 ;
	}
	return -EEXIST;
}



/*long mbx421_acl_remove(unsigned int id, pid_t process_id): 
Removes the PID specified by process_id from the ACL for the mailbox with the ID of id if it 
exists and the user has access to do so. Returns 0 on success or an appropriate error code on failure.
Only the root user shall be allowed to call this function
.
*/

SYSCALL_DEFINE2(mbx421_acl_remove, unsigned int , id,pid_t ,process_id){
	int ret;
	struct MailBox * mailbox;
	printk("mbx421_acl_remove\n");
	mailbox = find_MailBox(container, id);
	if(mailbox == NULL)
		return -ENOENT ;
	ret = deleteElement_ACL(mailbox->acl, process_id);
	if (ret)
	{
		return 0 ;
	}

	return -EEXIST;
}

