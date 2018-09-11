#!/usr/bin/env bash

for url in \
    	'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_01/hs_ref_GRCh38.p12_chr1.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_02/hs_ref_GRCh38.p12_chr2.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_03/hs_ref_GRCh38.p12_chr3.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_04/hs_ref_GRCh38.p12_chr4.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_05/hs_ref_GRCh38.p12_chr5.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_06/hs_ref_GRCh38.p12_chr6.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_07/hs_ref_GRCh38.p12_chr7.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_08/hs_ref_GRCh38.p12_chr8.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_09/hs_ref_GRCh38.p12_chr9.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_10/hs_ref_GRCh38.p12_chr10.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_11/hs_ref_GRCh38.p12_chr11.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_12/hs_ref_GRCh38.p12_chr12.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_13/hs_ref_GRCh38.p12_chr13.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_14/hs_ref_GRCh38.p12_chr14.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_15/hs_ref_GRCh38.p12_chr15.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_16/hs_ref_GRCh38.p12_chr16.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_17/hs_ref_GRCh38.p12_chr17.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_18/hs_ref_GRCh38.p12_chr18.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_19/hs_ref_GRCh38.p12_chr19.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_20/hs_ref_GRCh38.p12_chr20.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_21/hs_ref_GRCh38.p12_chr21.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_22/hs_ref_GRCh38.p12_chr22.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_X/hs_ref_GRCh38.p12_chrX.fa.gz' \
		'ftp://ftp.ncbi.nih.gov/genomes/H_sapiens/CHR_Y/hs_ref_GRCh38.p12_chrY.fa.gz'
do
	cmd="curl --remote-name $url"
	echo $cmd
	$cmd
done
