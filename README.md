# cpp-editing-utils

Sometimes, API changes can affect such a large number of files that changing it all manually isn't feasible. If the changes are too significant, find&replace can't do the job even with use of regular expressions.

Although some complex `awk` use or some Python script can do it, there's no guarantee a C++ developer knows to use these tools. This is a set of C++ functions that should allow to make complex automatic changes to a C++ source code and to test it by comparing it to a manually refactored file.

These functions certainly don't solve all problems that may happen during refactoring, these are just ones that I needed. Feel free to add new ones and make a merge request from it.

Some functions with less obvious purposes are described in comments.
