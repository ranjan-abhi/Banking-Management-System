# Banking-Management-System
Networks Lab Repo

A simple Banking Management System using unix sockets

## How to Run program
* Command to compile the server and client
```
gcc server.c -o server

gcc client.c -o client
```
* Run the server in one terminal instance with the port as the first argument
```
./server $PORT
```
* Run the client in a different terminal instance with the IP and port of the server instance as its arguments.
```
./client $IP $PORT
```
For local debug environment IP = 127.0.0.1
* Create a login credentials file named ***login_file***.

 The format of an example *login_file* is given below
		Username Password Auth_Type
where AUTH_TYPE can be **A** for admin, **P** for police and **C** for customers.


Format of a customer file is as given below
		Date and Time                 Type      Amount Credited/Debited         Current Balance

where Date and time represents the date and time of the corresponding transaction, type is either debit or credit, Amount is the 		transaction amount, and current balance available after the transaction.

Assumptions-
	(1) Mini statement has 5 entries representing the recent 5 transaction with the following format
		Date and Time                 Type      Amount Credited/Debited         Current Balance
 	where Date and time represents the date and time of the corresponding transaction, type is either debit or credit, Amount is the 		transaction amount, and current balance available after the transaction. 
