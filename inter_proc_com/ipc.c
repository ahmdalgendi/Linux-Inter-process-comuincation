

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
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
typedef struct ACLNode
{

	// MAIL Box Id
	unsigned int process_id;

	// Array to hold pointers to node of different level
	struct ACLNode **node;

}ACLNode;

ACLNode * init_ACLNode(unsigned int key, int level)
{
	//use kmalloc

	ACLNode * ret = (ACLNode*)kmalloc(sizeof(ACLNode), GFP_KERNEL);

	ret->process_id = key;
	// Allocate memory to node
	//use kmalloc

	ret->node = (ACLNode**)kmalloc(sizeof(ACLNode*) * (level + 1), GFP_KERNEL);

	// Fill node array with 0(NULL)
	memset(ret->node, 0, sizeof(ACLNode*)*(level + 1));
	return  ret;
}

// Class for Skip list
typedef struct ACLSkipList
{
	int MAXLVL;
	//prob for adding new level to the node
	int P;
	//number of levels
	int num_level;

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
	ret.num_level = 0;
	ret.size = 0;
	// create header node and initialize process_id to -1 to be the minus infinity node
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


	while (r < 32768 / skipList->P && lvl < skipList->MAXLVL)
	{
		lvl++;
		r = generate_random_int();
	}

	return lvl;
}

// create new node
ACLNode* create_node_acl(unsigned int key, int level)
{
	ACLNode *n = init_ACLNode(key, level);
	return n;
}

// Insert given process_id in skip list
int insertElement_ACL(ACLSkipList* skipList, unsigned int proc_id)
{
	ACLNode* acl_node;
	int idx;
	int i;
	ACLNode **array_of_nodes;

	ACLNode *temp_node;
	temp_node = skipList->header;

	// create array of size MAXLVL + 1 for the new node
	idx = (skipList->MAXLVL) + 1;

	array_of_nodes = (ACLNode**)kmalloc(sizeof(ACLNode*) * idx, GFP_KERNEL);
	memset(array_of_nodes, 0, sizeof(ACLNode*)*(skipList->MAXLVL + 1));

	/*
	 * search for the position to be inserted
	 */
	for (i = skipList->num_level; i >= 0; i--)
	{
		while (temp_node->node[i] != NULL && temp_node->node[i]->process_id < proc_id)
		{
			temp_node = temp_node->node[i];
		}
		array_of_nodes[i] = temp_node;
	}

	/* reached level 0 and node pointer to
	right, which is desired position to
	insert process_id.
	*/
	temp_node = temp_node->node[0];


	/*if node is not the same Id or it is null
	 * then insert it
	 * otherwise node already inserted
	 */
	if (temp_node == NULL || temp_node->process_id != proc_id)
	{
		// Generate a random level for node
		int rand_level = randomLevel_ACL(skipList);

		
		if (rand_level > skipList->num_level)
		{
			for (i = skipList->num_level + 1;i < rand_level + 1;i++)
			{
				array_of_nodes[i] = skipList->header;
			}

			// set the level of the new node
			skipList->num_level = rand_level;
		}

		// create new node with random level generated
		acl_node = create_node_acl(proc_id, rand_level);

		// insert node
		for (i = 0;i <= rand_level;i++)
		{
			acl_node->node[i] = array_of_nodes[i]->node[i];
			array_of_nodes[i]->node[i] = acl_node;
		}
		return 1; // insertion completed
	}
	return 0; // insertion failed
}

// Display the list_ level by level using printk
void displayList_ACL(ACLSkipList * skipList)
{
	int i;

	printk("\n*****ACL Skip List*****\n");
	for (i = 0;i <= skipList->num_level;i++)
	{
		ACLNode *node = skipList->header->node[i];
		printk("Level %d: ", i);

		while (node != NULL)
		{
			printk("%d ", node->process_id);
			node = node->node[i];
		}
		printk("\n");
	}


}

// Display skip list level wise
void destroy_ACL(ACLSkipList *skipList)
{

	int len, i, a;
	ACLNode ** todel;
	ACLNode *node;

	len = 0;
	for (i = 0;i <= skipList->num_level;i++)
	{
		ACLNode *node = skipList->header->node[i];


		while (node != NULL)
		{
			node = node->node[i];
			len++;
		}
	}

	todel = (ACLNode**)kmalloc(sizeof(ACLNode*) * len, GFP_KERNEL);
	a = 0;
	for (i = 0;i <= skipList->num_level;i++)
	{
		node = skipList->header->node[i];
		while (node != NULL)
		{
			todel[a] = node;
			node = node->node[i];

			a++;
		}
	}

	for (i = 0;i < len;i++)
	{
		kfree(todel[i]);
	}
}

int delete_element_acl(ACLSkipList * skipList, unsigned int proc_id)
{
	int i;

	ACLNode *temp_node;

	// create update array and initialize it
	ACLNode **update;
	temp_node = skipList->header;

	// create update array and initialize it
	update = (ACLNode**)kmalloc(sizeof(ACLNode*) *  (skipList->MAXLVL + 1), GFP_KERNEL);
	memset(update, 0, sizeof(ACLNode*)*(skipList->MAXLVL + 1));


	for (i = skipList->num_level; i >= 0; i--)
	{
		while (temp_node->node[i] != NULL && temp_node->node[i]->process_id < proc_id)
		{
			temp_node = temp_node->node[i];
		}
		update[i] = temp_node;
	}
	/*found the level of the node */
	temp_node = temp_node->node[0];

	// If temp node is the one to be deleted
	if (temp_node != NULL && temp_node->process_id == proc_id)
	{
		
		for (i = 0;i <= skipList->num_level;i++)
		{
			
			if (update[i]->node[i] != temp_node)
					break;

			update[i]->node[i] = temp_node->node[i];
		}

		// Remove levels having no elements
		while (skipList->num_level > 0 && skipList->header->node[skipList->num_level] == 0)
		{
			(skipList->num_level)--;
		}
		return 1; // found and deleted
	}
	return 0; // not found, so nothing deleted
}

int search_element_acl(ACLSkipList * skipList, unsigned int key)
{
	ACLNode *temp_node;
	int i;
	temp_node = skipList->header;

	/*loop from the highest level*/
	for (i = skipList->num_level; i >= 0; i--)
	{
		while (temp_node->node[i] &&
			temp_node->node[i]->process_id < key)
			temp_node = temp_node->node[i];

	}

	/*now at the lowest level*/
	temp_node = temp_node->node[0];

	//check if the node is what we looking for
	if (temp_node && temp_node->process_id == key)
		return 1;  //found
	return 0; // notfound
}



// MailBox using liked lists to save the messages in each mail-box
struct MLNode {
	unsigned char * msg;
	long  msg_len;
	struct MLNode* next;
};

// The MAil box queue to save messages 
struct MailBox {
	//front stores the front node of LL and rear stores the last node of Queue
	struct MLNode *front, *rear;
	int size;
	ACLSkipList acl;
	long long id;
};


struct MLNode* new_mail_box_node(const unsigned char* msg, long len)
{    //use kmalloc

	struct MLNode* temp;
	int i;
	temp = (struct MLNode*)kmalloc(sizeof(struct MLNode), GFP_KERNEL);
	//use kmalloc
	temp->msg = msg;
	temp->msg_len = len;
	temp->next = NULL;
	return temp;
}

struct MailBox *createMailBox(int mxLevel, int p, int ID)
{
	struct MailBox *q;
	q = (struct MailBox *) kmalloc(sizeof(struct MailBox), GFP_KERNEL);
	q->front = q->rear = NULL;
	q->size = 0;
	q->acl = init_ACLSkipList(mxLevel, p);
	q->id = ID;
	return q;
}

// push a new message to the mailbox
void pushMailBox(struct MailBox *q, const unsigned char *msg, long len)
{
	//Create a new LL node
	struct MLNode* temp;
	temp = new_mail_box_node(msg, len);
	printk("now at pushMailBox , msg =  ");
	for (int i = 0; i < len; ++i)
	{
		printk("%c", msg[i]);
	}
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

//remove a node from the queue and return it
struct MLNode* popMailBox(struct MailBox *q)
{
	struct MLNode* temp;

	// If queue is empty, return NULL.
	if (q->front == NULL)
		return NULL;
	(q->size)--;

	temp = q->front;


	q->front = q->front->next;

	if (q->front == NULL)
		q->rear = NULL;
	return temp;
}

int size_mail_box(struct MailBox* q)
{
	return q->size;
}

void destroy_mail_box(struct MailBox *q)
{
	struct MLNode * node, *tmp;
	node = q->front;
	while (node != NULL)
	{
		tmp = node;
		node = node->next;
		kfree(tmp->msg);
		kfree(tmp);
	}
}






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
	ret = (MailBoxSkipListNode*)kmalloc(sizeof(MailBoxSkipListNode), GFP_KERNEL);

	ret->mailBox = mailBox;
	// Allocate memory to node
	//use kmalloc

	ret->forward = (MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) * (level + 1), GFP_KERNEL);

	// Fill node array with 0(NULL)
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
	int probs[5], found, i;
	MailBoxSkipList *ret;
	struct MailBox *mailBox;
	probs[0] = 2;
	probs[1] = 4;
	probs[2] = 8;
	probs[3] = 16;
	found = 0;
	for (i = 0; i < 4; ++i) {
		if (P == probs[i])
			found++;
	}
	if (!found || MAXLVL < 1)
		return NULL;

	ret = (MailBoxSkipList*)kmalloc(sizeof(MailBoxSkipList), GFP_KERNEL);
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
	int rnd;
	int new_lvl;
	rnd = generate_random_int();
	new_lvl = 0;
	//max number returned by generate_random_int() is 32768
	while (rnd < 32768 / skipList->P && new_lvl < skipList->MAXLVL)
	{
		new_lvl++;
		rnd = generate_random_int();
	}

	return new_lvl;
}

MailBoxSkipListNode* createNode_MailBoxNode(struct MailBox * mailBox, int level)
{
	return init_MailBoxSkipListNode(mailBox, level);
}

// Insert given process_id in skip list
int insert_element_mail_box_skip_list(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *node;
	MailBoxSkipListNode **skip_list_node_arr;
	struct MailBox *mailBox;
	int i;
	int rand_level;

	MailBoxSkipListNode* mail_box_skip_list_node;

	node = skipList->header;
	
	// create array of size MAXLVL + 1 for the new node
	skip_list_node_arr =
		(MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*)* (skipList->MAXLVL + 1), GFP_KERNEL);
	memset(skip_list_node_arr, 0, sizeof(MailBoxSkipListNode*)*(skipList->MAXLVL + 1));

	mailBox = createMailBox(skipList->level, skipList->P, key);

	/*
	 * search for the position to be inserted
	 */
	for (i = skipList->level; i >= 0; i--)
	{
		while (node->forward[i] != NULL &&
			node->forward[i]->mailBox->id < key)
			node = node->forward[i];
		skip_list_node_arr[i] = node;
	}

	/* reached level 0 and node pointer to
	right, which is desired position to
	insert new mail box.
	*/
	node = node->forward[0];

	/*if node is not the same Id or it is null
	 * then insert it
	 * otherwise node already inserted
	 */
	if (node == NULL || node->mailBox->id != key)
	{
		rand_level = randomLevel_MailBox(skipList);

		if (rand_level > skipList->level)
		{
			for (i = skipList->level + 1;i < rand_level + 1;i++)
				skip_list_node_arr[i] = skipList->header;

			// set the level of the new node
			skipList->level = rand_level;
		}

		// create new node with random level generated
		mail_box_skip_list_node = createNode_MailBoxNode(mailBox, rand_level);

		// insert node by rearranging pointers
		for (i = 0;i <= rand_level;i++)
		{
			mail_box_skip_list_node->forward[i] = skip_list_node_arr[i]->forward[i];
			skip_list_node_arr[i]->forward[i] = mail_box_skip_list_node;
		}
		return 1; // insertion completed
	}
	return 0; // insertion failed
}

// Display the list_ level by level using printk
void displayList_MailBoxSkipList(MailBoxSkipList *skip_list)
{
	MailBoxSkipListNode *node;
	int i;
	printk("\n*****Skip List*****\n");

	for (i = 0;i <= skip_list->level;i++)
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
void destroy_mail_box_skip_list(MailBoxSkipList *skipList)
{

	int len, i, a;
	MailBoxSkipListNode *node;
	MailBoxSkipListNode ** todel;
	len = 0;
	for (i = 0;i <= skipList->level;i++)
	{
		node = skipList->header->forward[i];


		while (node != NULL)
		{
			node = node->forward[i];
			len++;
		}
	}

	todel = (MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) * len, GFP_KERNEL);
	a = 0;
	for (i = 0;i <= skipList->level;i++)
	{
		node = skipList->header->forward[i];
		while (node != NULL)
		{
			todel[a] = node;
			node = node->forward[i];

			a++;
		}
	}

	for (i = 0;i < len;i++)
	{
		destroy_mail_box(todel[i]->mailBox);
		kfree(todel[i]);
	}
}

// Delete element from skip list
int deleteElement_MailBoxSkipList(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *temp_node;

	// create update array and initialize it

	MailBoxSkipListNode **skip_list_node_arr;
	int i;
	temp_node = skipList->header;

	// create update array and initialize it

	skip_list_node_arr =
		(MailBoxSkipListNode**)kmalloc(sizeof(MailBoxSkipListNode*) *   (skipList->MAXLVL + 1), GFP_KERNEL);
	memset(skip_list_node_arr, 0, sizeof(MailBoxSkipListNode*)*(skipList->MAXLVL + 1));



	for (i = skipList->level; i >= 0; i--)
	{
		while (temp_node->forward[i] != NULL &&
			temp_node->forward[i]->mailBox->id < key)
			temp_node = temp_node->forward[i];
		skip_list_node_arr[i] = temp_node;
	}

	/*found the level of the node */
	temp_node = temp_node->forward[0];

	// If temp node is the one to be deleted

	if (temp_node != NULL && temp_node->mailBox->id == key)
	{
		
		for (i = 0;i <= skipList->level;i++)
		{
		
			if (skip_list_node_arr[i]->forward[i] != temp_node)
				break;

			skip_list_node_arr[i]->forward[i] = temp_node->forward[i];
		}

		// Remove empty levels
		while (skipList->level > 0 &&
			skipList->header->forward[skipList->level] == 0)
			(skipList->level)--;
		return 1; // found and deleted
	}
	return 0; // not found, so nothing deleted
}

int search_element_mail_box_skip_list(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *temp_node;
	int i;
	temp_node = skipList->header;

	
	/*loop from the highest level*/
	for (i = skipList->level; i >= 0; i--)
	{
		while (temp_node->forward[i] &&
			temp_node->forward[i]->mailBox->id < key)
			temp_node = temp_node->forward[i];

	}

	/*now at the lowest level*/
	temp_node = temp_node->forward[0];

	//check if the node is what we looking for
	if (temp_node && temp_node->mailBox->id == key)
		return 1;  //found
	return 0; // notfound
}


struct MailBox * find_mail_box_and_return_node(MailBoxSkipList *skipList, unsigned int key)
{
	MailBoxSkipListNode *temp_node;
	int i;
	temp_node = skipList->header;

	
	for (i = skipList->level; i >= 0; i--)
	{
		while (temp_node->forward[i] &&
			temp_node->forward[i]->mailBox->id < key)
			temp_node = temp_node->forward[i];

	}

	/*now at the lowest level*/
	temp_node = temp_node->forward[0];

	//check if the node is what we looking for
	if (temp_node && temp_node->mailBox->id == key)
		return temp_node->mailBox;  //found
	return NULL; // notfound

}

int get_msgCount(MailBoxSkipList * skip_list, unsigned int key)
{
	struct MailBox * mail;
	mail = find_mail_box_and_return_node(skip_list, key);

	if (mail == NULL)
		return -1; // invalid key
	return mail->size;
}

//TODO
long mbx421_send_helper(MailBoxSkipList* skip_list, unsigned int id, const unsigned char
	*msg, long len)
{
	struct MailBox * mail;

	mail = find_mail_box_and_return_node(skip_list, id);
	if (mail == NULL)
	{
		return 0; // failed
	}
	printk("at mbx421_send_helper and mail was found\n");
	pushMailBox(mail, msg, len);
	return  1;
}
//TODO

long mbx421_recv_helper(MailBoxSkipList * skip_list, unsigned int id, unsigned char  *msg, long
	len)
{
	struct MailBox *ret;
	struct MLNode * poped;

	int cpd;

	ret = find_mail_box_and_return_node(skip_list, id);
	poped = popMailBox(ret);
	len = min(len, poped->msg_len);
	cpd = __copy_to_user(msg, poped->msg, len);
	return cpd;
}


MailBoxSkipList *container;
spinlock_t lock;
static int already_init;
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
SYSCALL_DEFINE2(mbx421_init, unsigned int, ptrs, unsigned int, prob) {
	struct timespec ts;
	printk("mbx421_init\n");
	
	if (get_current_cred()->uid.val != 0)
		return -EACCES;
	if (already_init)
		return  -EADDRINUSE;	
	getnstimeofday(&ts);
	seed_random(ts.tv_nsec);

	container = init_MailBoxSkipList(ptrs, prob);

	if (container == NULL)
		return -EINVAL;
	already_init = 1;
	
	spin_lock_init(&lock);

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
	spin_lock_irq(&lock);
	if (already_init)
	{
		return -ENOENT;
	}
	if (get_current_cred()->uid.val != 0)
	{
		spin_unlock_irq(&lock);
		return -EACCES;
	}
	if (container == NULL)
	{
		spin_unlock_irq(&lock);

		return -EPERM;
	}
	destroy_mail_box_skip_list(container);
	spin_unlock_irq(&lock);
	already_init = 0;
	return 0;
}

/*long mbx421_create(unsigned int id):
Creates a new mailbox with the given id if it does not already exist
(no duplicates are allowed). Returns 0 on success or an appropriate error on failure.
If an id of 0 or (264 - 1) is passed,
this is considered an invalid ID and an appropriate error shall be returned.
Only the root user shall be allowed to call this function.
*/
SYSCALL_DEFINE1(mbx421_create, unsigned int, id)
{
	int ret;
	spin_lock_irq(&lock);

	printk("mbx421_create\n");
	if (get_current_cred()->uid.val != 0)
	{
		spin_unlock_irq(&lock);
		return -EACCES;

	}
	 ret = insert_element_mail_box_skip_list(container, id);
	if (ret)
	{
		spin_unlock_irq(&lock);

		return 0;
	}
	spin_unlock_irq(&lock);

	return -EEXIST;
}

/*long mbx421_destroy(unsigned int id):
Deletes the mailbox identified by id if it exists and the user has permission to do so.
If the mailbox has any messages stored in it, these messages should be deleted.
Returns 0 on success or an appropriate error code on failure.
Only the root user shall be allowed to call this function.
*/

SYSCALL_DEFINE1(mbx421_destroy, unsigned int, id) {
	int ret;
	spin_lock_irq(&lock);

	printk("mbx421_destroy\n");
	if (get_current_cred()->uid.val != 0)
	{
		spin_unlock_irq(&lock);

		return -EACCES;
	}
	ret = deleteElement_MailBoxSkipList(container, id);
	spin_unlock_irq(&lock);

	if (ret)
		return 0;
	return -ENOENT;
}

/*long mbx421_count(unsigned int id):
Returns the number of messages in the mailbox identified by id if it
exists and the user has permission to access it.
Returns an appropriate error code on failure.
*/

SYSCALL_DEFINE1(mbx421_count, unsigned int, id) {
	printk("mbx421_count\n");
	int ret = get_msgCount(container, id);
	if (ret >= 0)
		return ret;
	return -ENOENT;

}

/*long mbx421_send(unsigned int id, const unsigned char __user *msg, long len):
Sends a new message to the mailbox identified by id if it exists and the user has access to it.
The message shall be read from the user-space pointer msg and shall be len bytes long.
Returns 0 on success or an appropriate error code on failure.
*/
SYSCALL_DEFINE3(mbx421_send, unsigned int, id, const unsigned char __user, *msg, long, len) {
	unsigned char * kmesg;
	int ret, num;
	printk("mbx421_send\n");
	if (msg == NULL || len <= 0)
		return -EINVAL;

	if(!access_ok(VERIFY_READ, msg , sizeof(char) * len)
	{
		return -EINVAL;
	}

	kmesg = (unsigned char *)kmalloc(sizeof(char) * len,GFP_KERNEL);

	num = __copy_from_user(kmesg, msg, len * sizeof(char));
	printk("iam after __copy_from_user mbx421_send\n");

	ret = mbx421_send_helper(container, id, kmesg, len);
	if (ret)
	{
		return num;
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

SYSCALL_DEFINE3(mbx421_recv, unsigned int, id, unsigned char __user, *msg, long, len) {
	
	int ret;
	printk("mbx421_recv\n");

	if (msg == NULL || len <= 0)
		return -EINVAL;

	if(!access_ok(VERIFY_WRITE, msg , sizeof(char) * len)
	{
		return -EINVAL;
	}
	
	ret = mbx421_recv_helper(container, id, msg, len);
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

SYSCALL_DEFINE1(mbx421_length, unsigned int, id) {
	int ret;
	printk("mbx421_length\n");
	ret = get_msgCount(container, id);
	if (ret != -1)
		return ret;

	return -ENOENT;
}


/*long mbx421_acl_add(unsigned int id, pid_t process_id):
Adds PID specified by process_id to the ACL for the mailbox with the ID of id if it
exists and the user has access to do so. Returns 0 on success or an appropriate error code on failure.
Only the root user shall be allowed to call this function.
*/


SYSCALL_DEFINE2(mbx421_acl_add, unsigned int, id, pid_t, process_id) {
	int ret;
	struct MailBox * mailbox;
	printk("mbx421_acl_add\n");
	mailbox = find_mail_box_and_return_node(container, id);
	if (mailbox == NULL)
		return -ENOENT;
	ret = insertElement_ACL(&mailbox->acl, process_id);
	if (ret)
	{
		return 0;
	}
	return -EEXIST;
}



/*long mbx421_acl_remove(unsigned int id, pid_t process_id):
Removes the PID specified by process_id from the ACL for the mailbox with the ID of id if it
exists and the user has access to do so. Returns 0 on success or an appropriate error code on failure.
Only the root user shall be allowed to call this function
.
*/

SYSCALL_DEFINE2(mbx421_acl_remove, unsigned int, id, pid_t, process_id) {
	int ret;
	struct MailBox * mailbox;
	printk("mbx421_acl_remove\n");
	mailbox = find_mail_box_and_return_node(container, id);
	if (mailbox == NULL)
		return -ENOENT;
	ret = delete_element_acl(&mailbox->acl, process_id);
	if (ret)
	{
		return 0;
	}

	return -EEXIST;
}
																											