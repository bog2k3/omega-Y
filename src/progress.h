#ifndef PROGRESS_H
#define PROGRESS_H

struct Progress {
	unsigned completed = 0;
	unsigned total = 0;
};

// type of a function that performs its opration in several steps, as opposed to all at once;
// this allows tracking of progress of a lengthy operation.
// the function is required to perform the step given as argument and return a description of where it left off
using progressiveFunction = Progress(*)(unsigned step);

#endif // PROGRESS_H
