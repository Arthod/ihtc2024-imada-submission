
# SDU-IMADA Entry to the IHTC 2024

The code in this repository implements the solver submitted to
[IHTC-2024](https://ihtc2024.github.io/ "IHTC-2024") competition by the team
SDU-IMADA.

The members of the team are:
- [Ahmad Mahir Othman](https://github.com/Arthod "Ahmad Mahir Othman")
- [Marco Chiarandini](https://github.com/belzebuu "Marco Chiarandini")

At the time of the competition both members were affiliated to the Department of
Mathematics and Computer Science (IMADA) of the University of Southern Denmark
(SDU).

Out of 32 submissions in total to the competition, the submission by SDU-IMADA
ranked 1st in the open-source only track and 2nd overall.

## Brief Description

The C++ code is the submitted implementation of a heuristic algorithm for the
interated healthcare timetabling problem, which was object of the IHTC-2024
competition. The algorithm is based on a multi-neighborhood local search guided
first by simulated annealing and in the last phase of search by iterated local
search.  The algorithm is designed to be efficient and scalable, allowing it to
handle all different problem instances of the competition within the time limit
of 10 minutes on a common 2024 desktop machine utilizing a maximum of 4 threads.

The following documents provide more detailed descriptions of the algorithms and
its performance: 

- The short report submitted contextually to the code [pdf](./doc/report.pdf
  "pdf"), together with a [`.csv` file](./solutions/score.csv) reporting the
  best results on the public instances and the [certificates](./solutions/) of
  these solutions.

- The presentation given at the [EURO 2025 conference](https://euro2025leeds.uk/
  "EURO 2025") in Leeds.
  [Abstract](https://www.euro-online.org/conf/euro34/treat_abstract?frompage=search&paperid=2774
  "EURO 2025") and slides [pdf](./doc/euro2025-slides.pdf "pdf").

- An article to be sumbmitted to the Special Issue on the
  IHTC-2024 of the journal /Operations Research, Data Analytics and Logistics/.
  (In preparation.)


## Building and Running the Code

The code is written in C++ 20 and is tested on Linux systems. It can be
compiled using the included Makefile:
```
make
```
The Makefile is set up to compile the code with the GNU C++ compiler (g++) and
assumes that the source files are located in the `src/` directory. The object
files will be placed in the `build/obj/` directory, and the executable will be placed
in the `build/` directory.

The executeable can be run as follows with optional arguments:
```
./build/main [input=../data/ihtc2024_competition_dataset/i01.json] [seed=rnd] [output=out/sol_{...}.json] [debug_level=0]
```
where
- `input` is input file with i01.json as default. 
- `seed` is seed with random as default.
- `output` is location to write output file.
- `debug_level` the verbose level. See `config.h` for more info.

For any questions, please do not hesitate to contact the authors via GitHub or
open an issue in this repository.