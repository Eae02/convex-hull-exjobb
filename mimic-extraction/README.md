This folder contains scripts and instructions to create convex hull datasets using the MIMIC patient datasets.

The idea is to reproduce the computations made in the following paper: https://doi.org/10.3389%2Ffdata.2022.603429

Follow the steps below to create your own convex hull inputs:

- Gain access to https://physionet.org/content/mimiciv/2.2/ and/or https://physionet.org/content/mimiciii/1.4/
by creating an account on physionet and completing the CITI training course: https://physionet.org/about/citi-course/
    - There are demo datasets available but they will create very small outputs.

- Download tables D_ITEMS, PATIENTS, ICUSTAYS from the website. They are quite small. Place them in data/mimiciv/ or choose some other appropriate dataset name.

- If your computer has lots of space and ram, also download CHARTEVENTS and load into some SQL database locally. Consider the following guide: https://mimic.mit.edu/docs/gettingstarted/local
    - You can then run the ventilation_durations sql query locally and export to csv.

- Otherwise, use the cloud: 
    - Apply for Google Bigquery access to the data. This can be done at the bottom of https://physionet.org/content/mimiciv/2.2/ if you are credentialised. 

    - Sign up for free trail on Google Cloud Platform and create a project and a dataset for the purpose.

    - Navigate to the physionet dataset you want to use and copy the CHARTEVENTS table into your own project. The easiest way to find it is from the following link: https://console.cloud.google.com/bigquery?project=physionet-data&page=dataset&ws=!1m0

    - Run the ventilation_durations [sql query](sql-queries/) to create the table.

    - Run the chartevents_numeric [sql query](sql-queries/) to create a smaller chartevents table that only contains numeric values. This reduces the table size roughly 4 times.

    - Download created tables CHARTEVENTS_NUM and VENTDURATIONS from Google Bigquery by following this guide to download the data https://stackoverflow.com/a/37274820
    Chartevents will probably need to be sharded using the '*'-syntax. There is no need to unzip the data locally.

- Run mimicExtraction.ipynb to create intermediate tables.

- Run create_data_sets.py to create convex hull inputs.

