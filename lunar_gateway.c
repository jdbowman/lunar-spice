//ILUMIN, SINCPT, SUBPNT, SUBSLR are better
//Moon_ME ref frame is mean direction toward the Earth for the X axis and the mean
//rotation direction for the Z axis. Y completes the right-handed triad

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

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

int main (int argc, char** argv) {

	// Temp stuff used in calculations
  SpiceChar currentUTC[WORDSZ];
  SpiceDouble et;
  SpiceDouble lt;
	SpiceDouble rotateFromMoonMEtoJ2000[3][3];
	SpiceDouble rotateFromJ2000toMoonME[3][3];
  SpiceDouble rotateFromMoonMEtoRefSiteTopo[3][3];
  SpiceDouble posMoon_J2000[3]; //in km
  SpiceDouble posSun_J2000[3]; //in km
  SpiceDouble posSun_MoonME[3]; //in km
  SpiceDouble posGateway_J2000[3]; //in km
	SpiceDouble posGateway_MoonME[3]; //in km
  SpiceDouble posRefSite_MoonME[3]; //in km
  SpiceDouble vecMoonToGateway_J2000[3]; //in km
  SpiceDouble vecMoonToSun_J2000[3]; //in km
  SpiceDouble vecRefSiteToGateway_J2000[3]; //in km
  SpiceDouble vecRefSiteToGateway_MoonME[3]; //in km
  SpiceDouble vecRefSiteToGateway_Topo[3]; // in km
  SpiceDouble vecRefSiteToSun_J2000[3]; //in km
  SpiceDouble vecRefSiteToSun_MoonME[3]; //in km
  SpiceDouble vecRefSiteNormal_J2000[3];
  SpiceDouble vecRefSiteNormal_MoonME[3];
  SpiceDouble vecZ[3] = {0.0, 0.0, 1.0};
  SpiceDouble distRefSiteToGateway;
  SpiceDouble distEarthToGateway;

	// SpiceDouble zenithAngle;
  SpiceDouble azRefSiteToGateway_Topo;
  SpiceDouble elRefSiteToGateway_Topo;
  SpiceDouble raRefSiteToGateway_J2000;
  SpiceDouble decRefSiteToGateway_J2000;
  SpiceDouble raRefSiteToSun_J2000;
  SpiceDouble decRefSiteToSun_J2000;

  // Default values for configuration
  SpiceDouble lonlat[2] = {180.0, 0.0};
  //SpiceDouble alt = 1734.4; // in km
  SpiceInt stepSeconds = 3600;
	SpiceInt numSteps = 8760;
	SpiceChar startUTC_default[WORDSZ] = "1/1/2025 00:00:00";
  SpiceChar furnsh_default[FILESZ] = "lunar_gateway_furnsh.txt";
  SpiceChar output_default[FILESZ] = "lunar_gateway_output.csv";

  SpiceChar* startUTC = startUTC_default;
  SpiceChar* furnsh_filepath = furnsh_default;
  SpiceChar* output_filepath = output_default;

  int opt;

  while ((opt = getopt(argc, argv, "d:f:g:hs:l:n:o:")) != -1) {

    switch (opt) {

      case 'd':
        startUTC = optarg;
        break;

      case 'f':
        furnsh_filepath = optarg;
        break;

      case 'h':
        fprintf(stderr, ""
          "lunar_gateway:\n\n"
          "-d    start date in UTC ('MM/DD/YYYY hh:mm:ss', def: '1/1/2025 00:00:00')\n"
          "-f    full path to required furnsh file listing needed SPICE data (def: 'lunar_gateway_furnsh.txt')\n"
          "-h    print this help message\n"
          "-n    number of steps to process (must be integer, def:8760)\n"
          "-s    step time in seconds (must be integer, def: 3600)\n"
          "-l    lunar latitude of reference site (-90 to 90 deg, def: 0)\n"
          "-g    lunar longitude of reference site (-180 to 180 deg, def: -180)\n"
          "-o    full path to output CSV file (def: 'lunar_gateway_output.csv')\n");
        return 0;

      case 'n':
        numSteps = atoi(optarg);
        break;

      case 'o':
        output_filepath = optarg;
        break;

      case 's':
        stepSeconds = atoi(optarg);
        break;

      case 'l':
        lonlat[1] = atof(optarg);
        break;

      case 'g':
        lonlat[0] = atof(optarg);
        break;

      case '?':
        if (optopt == 'f' || optopt == 'o' || optopt == 't' || optopt == 'n')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;

      default:
        break;
    }
  }

  // Print command line configuration
  printf("SPICE Furnsh filepath: %s\n", furnsh_filepath);
  printf("Output CSV filepath: %s\n", output_filepath);
  printf("Start date (UTC): %s\n", startUTC);
  printf("Step size (seconds): %d\n", stepSeconds);
  printf("Number of steps: %d\n", numSteps);
  printf("Lunar latitude: %6.3f\n", lonlat[1]);
  printf("Lunar longitude: %6.3f\n", lonlat[0]);

  // convert lonlat to radians
  lonlat[0] *= rpd_c();
  lonlat[1] *= rpd_c();

  // Load SPCE kernels and stuff
  furnsh_c(furnsh_filepath);

	// Open output file
  FILE *file;
  file = fopen(output_filepath, "w");
	fprintf(file, "# UTC, Distance DSG-Earth, Distance DSG-RefSite, Az (deg), El (deg), RA (deg), Dec (deg)\n");

	// Convert the starting date to seconds past J2000 epoch
	str2et_c(startUTC, &et);

	// Loop over time steps
  for (int n=0; n<numSteps; n++) {
 
    // Calculate Sun, Moon, and Gateway (-60000) positions with Earth centered coordinates
    spkpos_c("SUN", et, "J2000", "NONE", "EARTH", posSun_J2000, &lt);
    spkpos_c("MOON", et, "J2000", "NONE", "EARTH", posMoon_J2000, &lt);
		spkpos_c("-60000", et, "J2000", "NONE", "EARTH", posGateway_J2000, &lt);

//    printf("J2000:  %6.3f, %6.3f, %6.3f\n", posMoon_J2000[0], posMoon_J2000[1], posMoon_J2000[2]);
//    printf("J2000:  %6.3f, %6.3f, %6.3f\n", posSun_J2000[0], posSun_J2000[1], posSun_J2000[2]);

    // Calculate matrix "rotate" that transforms position vectors from "Moon_ME"
		// to "J2000" at the current epoch and the opposite ("rotateFromJ2000toMoonME")
    pxform_c("MOON_ME", "J2000", et, rotateFromMoonMEtoJ2000);
		pxform_c("J2000", "MOON_ME", et, rotateFromJ2000toMoonME);

		// Convert target positions to Moon_ME coordinate frame
    vsub_c(posGateway_J2000, posMoon_J2000, vecMoonToGateway_J2000);
    vsub_c(posSun_J2000, posMoon_J2000, vecMoonToSun_J2000);
		mxv_c(rotateFromJ2000toMoonME, vecMoonToGateway_J2000, posGateway_MoonME);
		mxv_c(rotateFromJ2000toMoonME, vecMoonToSun_J2000, posSun_MoonME);

	  // Get MOON_ME coordinates of reference site on Moon (lat/lon/alt)
    // old: latrec_c(alt, lon*rpd_c(), lat*rpd_c(), posRefSite_MoonME);
    latsrf_c("Ellipsoid", "MOON", et, "MOON_ME", 1, &lonlat, &posRefSite_MoonME);

    // Get vector from reference site on Moon's surface to Gateway in MOON_ME coordinates
    vsub_c(posGateway_MoonME, posRefSite_MoonME, vecRefSiteToGateway_MoonME);
    vsub_c(posSun_MoonME, posRefSite_MoonME, vecRefSiteToSun_MoonME);

	  // Get normal vector to Moon's surface at current lat/lon
	  srfnrm_c("Ellipsoid", "MOON", et, "MOON_ME", 1, &posRefSite_MoonME, &vecRefSiteNormal_MoonME);

    // And vectors also get in J2000
    mxv_c(rotateFromMoonMEtoJ2000, vecRefSiteToGateway_MoonME, vecRefSiteToGateway_J2000);
    mxv_c(rotateFromMoonMEtoJ2000, vecRefSiteToSun_MoonME, vecRefSiteToSun_J2000);
    mxv_c(rotateFromMoonMEtoJ2000, vecRefSiteNormal_MoonME, vecRefSiteNormal_J2000);

//    printf("J2000:  %6.3f, %6.3f, %6.3f\n", vecRefSiteToGateway_J2000[0], vecRefSiteToGateway_J2000[1], vecRefSiteToGateway_J2000[2]);
//    printf("J2000:  %6.3f, %6.3f, %6.3f\n", vecRefSiteNormal_J2000[0], vecRefSiteNormal_J2000[1], vecRefSiteNormal_J2000[2]);
//    printf("J2000:  %6.3f, %6.3f, %6.3f\n", vecRefSiteNormal_MoonME[0], vecRefSiteNormal_MoonME[1], vecRefSiteNormal_MoonME[2]);


    // Calculate distance between Earth and Gateway
    distEarthToGateway = vnorm_c(posGateway_J2000);

    // Get a rotation between the Moon_ME and local topographics coordinates of the reference site
    twovec_c(vecRefSiteNormal_MoonME, 3, vecZ, 1, rotateFromMoonMEtoRefSiteTopo);

    // Calculate the azimuth and elevation of Gateway viewed from the reference site
    mxv_c(rotateFromMoonMEtoRefSiteTopo, vecRefSiteToGateway_MoonME, vecRefSiteToGateway_Topo);
    reclat_c(vecRefSiteToGateway_Topo, &distRefSiteToGateway, &azRefSiteToGateway_Topo, &elRefSiteToGateway_Topo);
    elRefSiteToGateway_Topo *= dpr_c();
    azRefSiteToGateway_Topo *= -1 * dpr_c();

    // Calculate the RA and Dec of Gatway viewed from the reference site
//    recrad_c(vecRefSiteToGateway_J2000, &distRefSiteToGateway, &raRefSiteToGateway_J2000, &decRefSiteToGateway_J2000);
    recrad_c(vecMoonToGateway_J2000, &distRefSiteToGateway, &raRefSiteToGateway_J2000, &decRefSiteToGateway_J2000);
    raRefSiteToGateway_J2000 *= dpr_c(); // now in degrees
    decRefSiteToGateway_J2000 *= dpr_c(); // now in degrees

    recrad_c(vecMoonToSun_J2000, &lt, &raRefSiteToSun_J2000, &decRefSiteToSun_J2000);
    raRefSiteToSun_J2000 *= dpr_c(); // now in degrees
    decRefSiteToSun_J2000 *= dpr_c(); // now in degrees

    // Write Gateway positions
  	et2utc_c(et, "ISOC", 0, WORDSZ, currentUTC);
		printf("Step=%d of %d (%s), d=%6.3f (km), el=%4.2f (deg)\n", n, numSteps, currentUTC, distRefSiteToGateway, elRefSiteToGateway_Topo);
		fprintf(file, "%s", currentUTC);
    fprintf(file, ", %6.3f", distEarthToGateway);
    fprintf(file, ", %6.3f", distRefSiteToGateway);
		fprintf(file, ", %4.2f", azRefSiteToGateway_Topo);
		fprintf(file, ", %4.2f", elRefSiteToGateway_Topo);
		fprintf(file, ", %4.2f", raRefSiteToGateway_J2000);
		fprintf(file, ", %4.2f", decRefSiteToGateway_J2000);
    fprintf(file, ", %6.3f", posMoon_J2000[0]);
    fprintf(file, ", %6.3f", posMoon_J2000[1]);
    fprintf(file, ", %6.3f", posMoon_J2000[2]);
    fprintf(file, ", %6.3f", posGateway_J2000[0]);
    fprintf(file, ", %6.3f", posGateway_J2000[1]);
    fprintf(file, ", %6.3f", posGateway_J2000[2]);
    fprintf(file, ", %6.3f", posGateway_MoonME[0]);
    fprintf(file, ", %6.3f", posGateway_MoonME[1]);
    fprintf(file, ", %6.3f", posGateway_MoonME[2]);
		fprintf(file, ", %4.2f", raRefSiteToSun_J2000);
		fprintf(file, ", %4.2f", decRefSiteToSun_J2000);

		fprintf(file, "\n");

		// Increment the epoch to the next step
		et += stepSeconds;
  }

	fclose(file);

}



