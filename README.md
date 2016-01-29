# Simple_GHS

# Description

The algorithm proposed is a simplified version of the pioneeristic distributed algorithm GHS([1]) for computing the minimum weight spanning tree (MST)  over a network. It is basically the result of the application of a synchronizer ([2]) to the traditional algorithm. The resulting algorithm (called Simple_GHS) has the same time complexity upper bound as GHS, namely O(n*log(n)), where n is the number of nodes of the network. Communication complexity is worse because of synchronization messages used at every level: now it is O(|E| * log(n)), where |E| is the number of nodes of the network.
A brief decription showing how it works is provided by the file Simple_GHS/source/algorithm_description.docx.


[1] Gallager, R.G., Humblet, P.A. and Spira, P.M.: A distributed algorithm for minimum weight spanning trees. ACM Transaction on Programming Languages and Systems, 55: 66-77, 1983.

[2] Boruch Awerbach. Complexity of network synchronization. Journal of the ACM. 35(4): 845-875, October 1988.

# Input
Input files are contained in the folder Simple_GHS/bin/Test/Input. Each file provides the adjacency matrix related to the input network.

# Output
 On standard output is printed the MST, which includes all the edges with their label (branch or rejected) and the index of the leader (root node of the MST). Furthermore, each node writes its own log files containing all the phases of the algorithm. These files are gathered in folder Simple_GHS/bin/Test/Log_files.
 

# Contents (source folder)

Edge: this class contains all the data associated to the edge connecting node i to node j.

File_ptr: RAII class for input and output files.

Message: this class wraps the messages that nodes exchange. A message is composed of message type, ID of sender node and a series of parameters represented by integer values.

Node: node class contains the protocol executed at a single node.

Output_edges: class necessary to output the MST.

split: split function that divide into substrings an input string containing words separated by one blank space.

Synch_queue: class that contains the queue of incoming messages associated to a node. Access to it is synchronized to avoid race conditions.

to: template function that just converts a type into another one.

# How to compile and run

I have executed and tested the program with Mac OS X using both CMake and Xcode. Here, for portability sake, I have included the cmakelists files necessary to compile the programs with CMake. To run the program with a different OS, you need just to provide slight modifications to file Simple_GHS/source/cmakelists.txt to take into account the difference in the file system.
To build the executable file, you should go to the directory where you want to store the file and type cmake followed by the path leading to the directory containing the source files. For building this program you have to go to the folder Simple_GHS/bin/simple_ghs and type:
 
"cmake path_to_folder_containing_the_project/Simple_GHS/source" 

This will generate the build files necessary for compilation. To compile the source files, type "make". Every time you edit  a source file you have to type "make" again to recompile the files. At this point, you need just to run the executable file to start the program.





