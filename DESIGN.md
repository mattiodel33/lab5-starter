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
