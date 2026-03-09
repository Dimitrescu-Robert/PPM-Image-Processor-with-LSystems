# Procedural Turtle Graphics & PPM Image Processor

This project is a C-based graphics engine that combines **L-Systems** (Lindenmayer systems) with **Turtle Graphics** to procedurally draw on **PPM (Portable Pixmap)** images. It features a custom timeline architecture that allows for robust UNDO/REDO functionality across different types of operations.

### Core Files:
`runic.c`, `lsys_functions.c`, `undoable_functions.c`, `turtle.c`, `bitcheck.c`, `header.h`

## Initialization (`runic.c` and `header.h`)

In `runic.c`, we initialize the variables used throughout the entire codebase. The main purpose of this file is to read user input from `stdin` and route it to the appropriate feature logic. The program ensures smooth execution and handles the freeing of dynamically allocated memory once it is no longer needed.

In `header.h`, we define all the structures used across different files, granting us the flexibility to use them globally. This file also stores the function signatures required across multiple modules.

## UNDO / REDO Logic

### Introduction
Throughout the execution, we process multiple *undoable* commands. Because these commands can be mixed, it presents a challenge for state management. 

### State Logs
The architecture of the undo/redo logic is implemented as follows: for every undoable command, we create a *log* entry storing that specific action. Because many of our commands involve modifying or adding a PPM image, we use **two main logs**: 
1. **L-system log:** Remembers all entries of this type alongside relevant data (axioms, number of rules, and the rules themselves).
2. **PPM log:** Stores relevant information about a PPM image (type, size, the pixel data, etc.).

### Command Timeline
To remember the exact order in which commands were called, we use an `action_timeline` array. This array memorizes **in chronological order** all undoable commands (stored via a character key) and their respective index in their specific log. This allows for rapid navigation of past actions via linear traversal. We dynamically reallocate memory every time we add a new command or *deviate* from the current timeline.

If we go back in time using UNDO and add a *new* command, **we no longer need the commands that existed after that UNDO point**, because we can no longer return to them. A good analogy is the *time travel paradox*: any change made from a point in the past completely alters the future, rendering the previous "future" irrelevant. Therefore, we discard the information we no longer need to save memory.

## L-System Implementation

### Reading and Storing an L-System
For every L-system command received, we use the `lsys_setup()` function to open the text file (read-only) containing the relevant data. We read and store it in an `lsys_file` structure, which holds:
* The name of the accessed file.
* The starting axiom (which is implicitly the 0-degree derivation).
* The number of derivation rules.
* The derivation rules themselves, stored as key->value pairs using a *hashmap* structure.

The advantage of this implementation is that we keep all the information in RAM for fast access. As mentioned above, each loaded L-system is also saved in the `past_lsys` log to enable UNDO/REDO actions. Upon success, the message `Loaded <file> (L-system with <n> rules)` is displayed.

### L-System Derivation
To derive an L-system, we first check if one is currently loaded. If not, we print: `No L-system loaded`. If one is available, we call `lsys_derivation` and follow these steps:

1. Calculate the maximum possible string length for the next derivation. We do this to allocate memory efficiently based on an analytical decision rather than guessing (we take the worst-case scenario and adjust it at the end).
2. Start the derivation loop until the desired degree is reached.
3. For each character in the last derivation, we look up a rule in the L-system *hashmap*. If found, we apply it and copy the result to the next derivation string; otherwise, we copy the character as is.
4. Once the loop finishes, we obtain the result and print it if requested. Because the derived string might be needed for later drawing operations, the result is returned separately from the display function.

## Turtle Graphics Implementation

This module handles three main functionalities: loading, modifying, and saving a PPM image. 

### Parsing PPM Images

#### Reading PPMs
The logic for reading images is handled by `ppm_setup` inside `turtle.c`. Since PPM files are binary, we open the file in *read-binary* mode. Even though it's a binary file, the image *header* is human-readable (ASCII). Therefore, we read the header using `fscanf`. The actual image data (the pixel values) are read using `fgetc`, as each value consists of a single byte (0 - 255) in binary format.

#### Storing PPMs
The data is stored in a `ppm_file` structure. Because each pixel has a specific 2D coordinate and contains three chromatic values (RGB), the most efficient way to store this data is via a pixel matrix. By reading the width and height beforehand, we calculate the exact image size and dynamically allocate the precise amount of memory needed. Pixel values are stored as integers (casting the `fgetc` result) for easier manipulation.
Just like the L-systems, every loaded image is recorded in the `ppm_log`.

### Modifying PPM Images
To draw on the canvas (using the `TURTLE` command), we must verify that both a PPM image and an L-system are loaded in the current timeline state using the `search_load()` function.

Drawing is executed by `turtle_draw` following these steps:
1. Verify the presence of an active L-system and PPM image.
2. Read drawing instructions from `stdin`.
3. Process the L-system derivation to generate the drawing path.
4. Initialize a `state` array to remember the Turtle's positions when required, dynamically reallocating memory as it grows.
5. Parse the instructions:
    * **`F`** -> Calculate the target endpoint using the current position and angle. We then apply **Bresenham's line algorithm** to draw the segment on the image matrix, updating the RGB triplets for each calculated point.
    * **`+`** -> Increase the angle by `angle_step` degrees.
    * **`-`** -> Decrease the angle by `angle_step` degrees.
    * **`[`** -> Save the Turtle's current position to the end of the `state` array.
    * **`]`** -> Revert to the Turtle's last saved position from the `state` array and remove it from the array.
6. Save the resulting image as the "current image" and add it to the PPM log, treating it as a new timeline iteration.

### Saving PPM Images
The `save_img` function handles the `SAVE` command. It takes the most recent image from memory and saves it under a new alias provided via `stdin`. After verifying the image exists in memory, we open a new file in binary mode, write the header using `fprintf`, and write the integer matrix data using `fwrite` to respect the binary PPM format.

## Bitcheck Utility

### Introduction
The `binary_bitcheck` function is designed to verify the bit-level integrity of the currently loaded image.

### Implementation
We first ensure an image is loaded using `search_load`. Due to how images are stored (integer pixel matrices), binary conversion is a simple base-10 to base-2 translation. Because errors might span across the bits of pixels on different rows/columns, we traverse the entire image pixel by pixel (verifying chunks of 4 bits) and concatenate their binary values into a single continuous string. The required string length is pre-calculated based on image dimensions to allocate the exact memory needed.

We scan this continuous bitstream for specific problematic sequences: **`1101`** and **`0010`**. Once found, we flip the problematic bit (the 3rd bit in the sequence, changing 0->1 or 1->0). With the bit flipped, the affected pixel's value is completely recalculated and cast back to an integer (`binary_transform`). After logging the pixel error, we revert the bit to its original state and continue the traversal to the end.