#!/usr/bin/env bash

data_dir=data
current_dir=$pwd

doc_urls=(
"https://ipnpr.jpl.nasa.gov/progress_report/42-178/178C.pdf"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de430_and_de431.pdf"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de430_moon_coord.pdf"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/aareadme_de430-de431.txt"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de430_tech-comments.txt"
"https://naif.jpl.nasa.gov/naif/lunar_kernels.txt"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/fk/planets/earth_assoc_itrf93.tf"
"https://naif.jpl.nasa.gov/pub/naif/LADEE/kernels/fk/moon_080317.tf"
"https://naif.jpl.nasa.gov/pub/naif/LADEE/kernels/fk/moon_assoc_me.tf")


data_urls=(
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de430.bsp"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/moon_pa_de421_1900-2050.bpc"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/lsk/a_old_versions/naif0008.tls"
"https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/pck00010.tpc"
"https://naif.jpl.nasa.gov/pub/naif/misc/MORE_PROJECTS/DSG/receding_horiz_3189_1burnApo_DiffCorr_15yr.bsp"
"https://naif.jpl.nasa.gov/pub/naif/misc/MORE_PROJECTS/DSG/nrho.lbl"
)

# Lunar Gateway kernels, see more at:
# https://naif.jpl.nasa.gov/pub/naif/misc/MORE_PROJECTS/DSG/
# https://s2e2.cosmos.esa.int/bitbucket/projects/SPICE_KERNELS/repos/lunar-gateway/browse
# https://ntrs.nasa.gov/api/citations/20190030294/downloads/20190030294.pdf

cd $data_dir

for url in ${doc_urls[@]}
do
  echo $url
  wget $url
done


for url in ${data_urls[@]}
do
  echo $url
  wget $url
done

cd $current_dir

