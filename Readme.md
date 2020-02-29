# New version of linux kernel with Inter Process Comunication
## in this project I implemented inter process communication protocole within the linux kernel

---------------
## New added system calls


* long mbx421_init(unsigned int ptrs, unsigned int prob): 

		Initializes the mailbox system, setting up the initial state of the skip list. The ptrs parameter specifies 
		the maximum number of pointers any node in the list will be allowed to have. The prob parameter specifies the 
		inverse of the probability that a node will be promoted to having an additional pointer in it 
		(that is to say that if the function is called with prob = 2, then the probability that the node will 
		have 2 pointers is 1 / 2 and the probability that it will have 3 pointers is 1 / 4, and so on). 
		The only valid values for the prob parameter are 2, 4, 8, and 16 — any other value shall r
		esult in an error being returned. Additionally, the ptrs parameter must be non-zero — a zero 
		value should result in an error being returned. Returns 0 on success. Only the root user 
		(the user with a uid of 0) shall be allowed to call this function.
* long mbx421_shutdown(void): 

		Shuts down the mailbox system, deleting all existing mailboxes and any messages contained 
		therein. Returns 0 on success. Only the root user shall be allowed to call this function.
* long mbx421_create(unsigned long id): 

		Creates a new mailbox with the given id if it does not already exist (no duplicates are allowed). 
		Returns 0 on success or an appropriate error on failure. If an id of 0 or (264 - 1) is passed, 
		this is considered an invalid ID and an appropriate error shall be returned. Only the root user 
		shall be allowed to call this function.
* long mbx421_destroy(unsigned long id): 

		Deletes the mailbox identified by id if it exists and the user has permission to do so. 
		If the mailbox has any messages stored in it, these messages should be deleted. 
		Returns 0 on success or an appropriate error code on failure. 
		Only the root user shall be allowed to call this function.
* long mbx421_count(unsigned long id):

		 Returns the number of messages in the mailbox identified by id if it exists and the user has permission to access it. 
		 Returns an appropriate error code on failure.
long mbx421_send(unsigned long id, const unsigned char __user *msg, long len): 
		
		Sends a new message to the mailbox identified by id if it exists and the user has access to it.

		The message shall be read from the user-space pointer msg and shall be len bytes long. 
		Returns 0 on success or an appropriate error code on failure.

* long mbx421_recv(unsigned long id, unsigned char __user *msg, long len): 

		Reads the first message that is in the mailbox identified by id if it exists and the user has access to it, 
		storing either the entire length of the message or len bytes to the user-space pointer msg, whichever is less. 
		The entire message is then removed from the mailbox (even if len was less than the total length of the message).
		 Returns the number of bytes copied to the user space pointer on success or an appropriate error code on failure.

* long mbx421_length(unsigned long id): 

		Retrieves the length (in bytes) of the first message pending in the mailbox identified by id, 
		if it exists and the user has access to it. Returns the number of bytes 
		in the first pending message in the mailbox on success, 
		or an appropriate error code on failure.
* long mbx421_acl_add(unsigned long id, pid_t process_id): 

		Adds PID specified by process_id to the ACL for the mailbox with the ID of 
		id if it exists and the user has access to do so. Returns 0 on success or an 
		appropriate error code on failure. Only the root user shall be allowed to call 
		this function.
* long mbx421_acl_remove(unsigned long id, pid_t process_id): 

		Removes the PID specified by process_id from the ACL for the mailbox with the 
		ID of id if it exists and the user has access to do so. Returns 0 on 
		success or an appropriate error code on failure. 
		Only the root user shall be allowed to call this function.


This project was part of the Operating system course from UMBC

[Full descreption to the project](https://bluegrit.cs.umbc.edu/~lsebald1/cmsc421-fa2019/project1.shtml)

## for how to run and compile

1. apt-get install build-essential qt5-default qtcreator valgrind nano patch diffutils curl fakeroot git pkg-config gedit libssl-dev libncurses5-dev libelf-dev bison flex bc
2. chmod 777 . // before cloning the repo
3. make mrproper
4. make xconfig
5. make localmodconfig
6. make bindeb-pkg 

		During the make xconfig step, you should go into the Library routines section of the configuration and 
		make the "CRC32c CRC algorithm" or "CRC32c (Castagnoli, et all) Cyclic Redundancy-Check" 
		option be built-in to the kernel instead of being built as a module (make it so it has a checkmark, 
			not a dot in the box next to it). Once you've done that, hit the floppy disk icon to save the 
			configuration. If the make localmodconfig step prompts you at any point, you can just hit 
			Enter to accept the defaults. If you have increased the number of cores given to your 
			VM earlier in the setup, you may wish to give the -j flag to the make bindeb-pkg 
			step to allow it to use multiple cores. If you've dedicated 2 cores to your VM, 
			you'd run that command as make -j2 bindeb-pkg instead.
