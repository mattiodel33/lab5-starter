1.) Each chat in my program takes roughly 1370 bytes according to valgrind, and so 10 chats would take 13700 bytes of memory.
100 chats would take 137000 bytes, and 1000 chats would take 1370000 bytes.

When handling the /chats request, no matter how many chats there are only 500 bytes of working memory is used
(assuming nobody has reacted, otherwise it changes to 650 working bytes)
although the total number of bytes used is 500 times the chat number (or 650 times the chat number if every single one has a reply).
Thus, the total number of bytes used would be 5000, 50000, and 500000 for 10, 100, and 1000 chats respectively,
although after every chat the loop would repeat and the memory would be freed for use again.


2.) One limit placed on the input is the order in which the variables must be put in.
For example, to add a chat the user would need to put in the username and message sequentially, not the other way around.
The same also goes for reaction. Therefore, removing this limit would make them happy as they wouldn't need to remember the order.
A problem resulting from this implementation is that more complex code would be needed to interpret the request,
regardless of the order the user uses. This results in more unreadable/complicated code and also more processing time
due to more cases that need to be handled. In my implementation specifically I would need to consider the cases where
the user does not put the request in the order as specified by the PA writeup.

3.) (RESUBMIT) One good thing about this alternate design is that it would be much easier to write all of the chats and reactions
to the user. Rather than having to access multiple parts of a struct you just print out the char pointer to the chat.
A bad thing about this alternate design however is that it is much harder to access specific parts of the chat.
Unlike having a struct, if you only have a single string you would either need to keep track of the exact indices
that each part of the chat starts at (id, user, message, etc.) for a static memory allocation design which would be tedious, or
you would need to have some way of keeping track of each index and constantly update them a dynamic memory allocation design,
which would also be tedious to program and would possibly require more memory.

4.) (RESUBMIT) This design would make it harder to add the /edit feature as for either the static or dynamic memory allocation design,
you would need to know the index of where the actual message starts (you wouldn't need the index of the id, since those are
being kept track of by the array of char pointers) and then you would have to change the string. You could also just create
a new string, but then you would need to find all of the other parts of information to copy them over too. For the dynamic
memory allocation design specifically it would be much harder to add the feature, as after changing message you would need
to shift the rest of the chat (only reactions follow the message so you'd only have to shift those), and then update their
respective indices.
