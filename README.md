# Project Title: Real-Time File Monitoring System

---

## Description:
This project implements a real-time file monitoring system consisting of a client and a server component. The client monitors changes in a specified data folder and its subfolders, sending the updated file information to the server. The server listens for incoming data from clients and logs the received file information along with the client's IP address.

---

## Client Component:

- Upon startup, the client sends file information from its data folder and subfolders to the server.
- After receiving acknowledgment from the server, the client begins monitoring changes in the data folder and subfolders.
- When a change occurs (such as file renaming, deletion, addition, or modification), the client captures the new tree path of the data and subfolders and sends it to the server.

## Server Component:

* The server listens on the specified port for incoming connections from clients.
* Upon receiving data from a client, the server logs the received file information in the "server_data.log" file.
* The log format includes the client's IP address followed by details of each file: filename, size, and last modification date.
* The server updates the file list if the client is still present in the log file.

---


##  Requirements 

To run this project you need :
    - gcc
    - pthread lib

## Usage Instructions:

- Clone the repository from GitHub.

```bash
git clone https://github.com/SergeNoah000/real-time-monitoring-client-server-in-c.git
```


- Navigate to the project directory.

```bash
cd real-time-monitoring-client-server-in-c
```


- Compile the project using the provided Makefile by running the command

```bash
make
``` 


- Execute the server by running 

```bash
./server_runner PORT
```

Where `PORT` is the desired port number.


- Execute the client by running 

```bash
./client_watch IP_SERVER PORT
```


Where `IP_SERVER` is the server's IP address and `PORT` is the server's port number.


- All client and server actions are logged in `client_log.log` and `server_log.log`, respectively. You can stop client and server by a `CTRL + C`


- To remove the executables, run 

```bash 
make clean
```

---

## Contributing: 
Contributions and suggestions for improvement are welcome. Please feel free to submit pull requests or raise issues on the GitHub repository.

---

## Acknowledgments:
This project was developed as part of the Master I program in Computer Science, specializing in Operating Systems and Networks, at the University of Yaoundé I (http://uy1.cm).

---

## License:
This project is licensed under the [UY1 License](LICENSE.md).

---

**Contact Information:**
For inquiries or assistance, please contact the project maintainer: [Serge Noah](mailto:gaetan.noah@facsciences-uy1.cm)

---

**Note:**
This README provides a brief overview of the project and usage instructions. For more detailed technical documentation, please refer to the project's codebase and additional documentation files.