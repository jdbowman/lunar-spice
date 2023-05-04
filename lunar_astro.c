// run with  gcc eqArrOverTime.c -o eqArrOverTime -I/Users/hegedus/Documents/SPICE/cspice/include /Users/hegedus/Documents/SPICE/cspice/lib/cspice.a -lm

// gcc –I/myhome/cspice/include -o demo demo.c /myhome/cspice/lib/cspice.a –lm

//ILUMIN, SINCPT, SUBPNT, SUBSLR are better


//Moon_ME ref frame is mean direction toward the Earth for the X axis and the mean
//rotation direction for the Z axis. Y completes the right-handed triad




#include <stdio.h>
#include "SpiceUsr.h"

#define FILESZ 256
#define WORDSZ 41

// AC a:  +219.9021  /   -60.8340
// AC b:  +219.8961  /   -60.8375
// PC:		+217.4290  /   -62.6795
// GC:	  +266.417 		/ 	-29.00781

#define RA_AlphaCentauri +219.9042  // deg
#define DEC_AlphaCentauri -60.8339  // deg

int main ()
{
	SpiceChar startUTC[WORDSZ] = "1/1/2025 00:00:00";
  SpiceChar currentUTC[WORDSZ];
  SpiceDouble et;
  SpiceInt stepSeconds = 3600;
	SpiceInt numSteps = 8760;

	// temp stuff used in loops
	SpiceDouble rotateFromMoonMEtoJ2000[3][3];
	SpiceDouble rotateFromJ2000toMoonME[3][3];
	SpiceDouble rectan[3];
  SpiceDouble lat, lon;  
  SpiceDouble lt;

  SpiceDouble lunarPos[3]; //in km
  SpiceDouble sunPos[3]; //in km
	SpiceDouble jupPos[3]; //in km

	SpiceInt numTargets = 5;
	SpiceDouble astroRaDec[3][2] = { {219.90, -60.83}, {217.43, -62.68}, {266.42, -29.01} }; // in deg
	SpiceDouble targetsPos[5][3]; //in km
	SpiceDouble targetsMoonME[5][3]; //in km
	SpiceDouble zenithAngle[5];

	SpiceInt numLats = 19;
	SpiceInt numLons = 1;
	SpiceDouble lunarLats[19] = {0, -5, -10, -15, -20, -25, -30, -35, -40, -45, -50, -55, -60, -65, -70, -75, -80, -85, -90};
	SpiceDouble lunarLons[1] = {90};
	SpiceInt locationID;

  SpiceInt npts = 1;
  SpiceDouble srfpt[1][3];
  SpiceDouble normal[1][3];

  // Load SPCE kernels and stuff
  furnsh_c("lunar_astro_furnsh.txt");

	// Open output file
  FILE *file;
  file = fopen("zenith_angles.csv", "w");
	fprintf(file, "UTC, Location ID, Lunar lat (deg), Lunar lon (deg), AlphaCen, ProxCen, Galaxy, Sun, Jupiter\n");

  // Calculate effective positions for astronomical targets
  for (int i=0;i<3; i++) {
		radrec_c(100000000000, astroRaDec[i][0]*rpd_c(), astroRaDec[i][1]*rpd_c(), targetsPos[i]);
		//printf("Target (%d) RA, DEC (deg): %6.3f, %6.3f\n", i, targetsRaDec[i][0], targetsRaDec[i][1]);
		//printf("Target (%d) position: %6.3f, %6.3f, %6.3f\n", i, targetsPos[i][0], targetsPos[i][1], targetsPos[i][2]);
	}

	// Convert the starting date to seconds past J2000 epoch
	str2et_c(startUTC, &et);

	// Loop over days and months
  for (int n=0; n<numSteps; n++) {
 
		// Increment the epoch to the next step
		et += stepSeconds;
		et2utc_c(et, "ISOC", 0, WORDSZ, currentUTC);
		printf("ET: %12.2f\n", et);

    // Calculate sun and moon position with earth centered coordinates
    spkpos_c("MOON", et, "J2000", "NONE", "EARTH", lunarPos, &lt);
    spkpos_c("SUN", et, "J2000", "NONE", "EARTH", targetsPos[numTargets-2], &lt);
		spkpos_c("5", et, "J2000", "NONE", "EARTH", targetsPos[numTargets-1], &lt);

    //printf ("Moon in J2000 %11.6f , %11.6f , %11.6f \n", lunarPos[0], lunarPos[1], lunarPos[2]);
    //printf ("Sun in J2000 %11.6f , %11.6f , %11.6f \n", sunPos[0], sunPos[1], sunPos[2]);
    //printf ("Jupiter in J2000 %11.6f , %11.6f , %11.6f \n", jupPos[0], jupPos[1], jupPos[2]);

    // Calculate matrix "rotate" that transforms position vectors from "Moon_ME"
		// to "J2000" at the current epoch and the opposite ("rotateFromJ2000toMoonME")
    pxform_c("MOON_ME", "J2000", et, rotateFromMoonMEtoJ2000);
		pxform_c("J2000", "MOON_ME", et, rotateFromJ2000toMoonME);

		// Convert target positions to Moon_ME coordinate frame
		for (int i=0; i<numTargets; i++) {
			mxv_c(rotateFromJ2000toMoonME, targetsPos[i], targetsMoonME[i]);
		}

		// Loop over latitude and longitude on the Moon's surface
		locationID = 0;
    for(int ilat=0; ilat<numLats; ilat++) {

			lat = lunarLats[ilat]*rpd_c();

			for (int ilon=0; ilon<numLons; ilon++) {
      
      	lon = lunarLons[ilon]*rpd_c();
				locationID++;

				printf("Lat, Lon on Moon: %6.3f, %6.3f\n", lat, lon);

	      // Get MOON_ME coordinates of lat/lon/alt and put in rectan
	      latrec_c(1737.4, lon, lat, rectan);

	      // Get normal vector to Moon's surface at current lat/lon
	      srfnrm_c( "Ellipsoid", "MOON", et, "MOON_ME", npts, rectan, normal);

				// Start output entry for this step
				fprintf(file, "%s, %d, %6.3f, %6.3f", currentUTC, locationID, lat*dpr_c(), lon*dpr_c());

				for (int i=0; i<numTargets; i++) {
					zenithAngle[i] = dpr_c()*vsep_c(normal, targetsMoonME[i]);
					printf("(%s) Target %d zenith angle: %6.3f\n", currentUTC, i, zenithAngle[i]);
					fprintf(file, ", %4.1f", zenithAngle[i]);
				}

				fprintf(file, "\n");
			}
    }
  }

	fclose(file);

}



