
# IHTC 2024 - Team SDU-IMADA Submission

Repository consisting of the submission of Team SDU-IMADA to the [IHTC-2024](https://ihtc2024.github.io/ "IHTC-2024") competition. The submission of Team SDU-IMADA ranked 1st in open-source only software and 2nd overall in the competition out of 32 submissions total. Team members:
- [Ahmad Mahir Othman](https://github.com/Arthod "Ahmad Mahir Othman")
- [Marco Chiarandini](https://github.com/belzebuu "Marco Chiarandini")

## Submission
Other than the code for the algorithm, the following were also submitted to the competition and are included in the repository:
- Short description of the algorithm
- Computed solutions for the public instances and a statistics file of these

## EURO 2025 in Leeds
During the [EURO 2025](https://euro2025leeds.uk/ "EURO 2025") conference in Leeds, we had the opportunity to present our submission in the Automated Timetabling stream in a [session](https://www.euro-online.org/conf/euro34/edit_session_cluster?sessid=515 "session") dedicated to the IHTC. Abstract for the talk can be found [here](https://www.euro-online.org/conf/euro34/treat_abstract?frompage=search&paperid=2774 "here") and the slides of the presentation here.

## How to run

All code is written in C++ 20 and can be compiled using the included Makefile. The executeable can then be run as follows, with optional arguments:
```
./main [input=../data/ihtc2024_competition_dataset/i01.json] [seed=rnd] [output=out/sol_{...}.json] [debug_level=0]
```
where
- `input` is input file with i01.json as default. 
- `seed` is seed with random as default.
- `output` is location to write output file.
- `debug_level` the verbose level. See `config.h` for more info.

For any questions, please do not hesitate to contact the authors.