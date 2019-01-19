#include <iostream>
#include <hash_set>
#include <set>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <math.h>

#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <algorithm>

#include "s_hull_pro.h"

/* copyright 2012 Dr David Sinclair
   david@s-hull.org
 
	program to compute Delaunay triangulation of a set of points.
 
 this code may not be published or distributed without the concent of the copyright holder.
 
 */

using namespace std;

bool pointSortPredicate(const Shx& a, const Shx& b)
{  
	if (a.r < b.r)
		return true;
	else if (a.r > b.r)
		return false;
	else if (a.c < b.c)
		return true;
	else
		return false;  
};

bool pointComparisonPredicate(const Shx& a, const Shx& b)
{
	return a.r == b.r && a.c == b.c;
}

#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{
	if( 1 ){
		cerr << "new delaunay triangulation method test" << endl;

		std::vector<Shx> pts, hull;
		Shx pt;
		srand(1);

		//for(int v=0; v<20000; v++){
		for(int v=0; v<100000; v++){
			pt.id = v;     
			pt.r = (float) rand(); // pts.txt
			pt.c = (float) rand();

			pts.push_back(pt);
		}

		std::sort(pts.begin(), pts.end(), pointSortPredicate);
		std::vector<Shx>::iterator newEnd = std::unique(pts.begin(), pts.end(), pointComparisonPredicate);
		pts.resize(newEnd - pts.begin());

		write_Shx(pts, "pts.txt");

		std::vector<Triad> triads;

		s_hull_pro( pts, triads);

		write_Triads(triads, "triangles.txt");   

		exit(0);
	}

	if( 0){

		//cerr << "reading points from " << argv[1] << endl;
		std::vector<Shx> pts2;

		int nump = read_Shx(pts2, "pts.txt");//argv[1]);
		cerr << nump << " points read" << endl;

		// check for duplicates.
		std::vector<int> dupes;
		int numd = de_duplicate( pts2, dupes);
		cerr << "duplicates filtered  " << numd << endl;

		std::vector<Triad> triads;
		int ts = s_hull_pro( pts2, triads);

		write_Triads(triads, "triangles.mat");
		cerr << "triangles written to triangles.mat" << endl;
	}

	exit(0);
}

   


