Just a simple player for the [2023 Code Cup game](https://www.codecup.nl/) of sudoku.

### Rules
A full description of all the rules can be found [here](https://www.codecup.nl/rules.php). But in short, the game starts with an empty sudoku grid. Two players take turn filling one of the empty cells with a number, following the sudoku rules. Each move should decrease the amount of solutions; and the first player to claim an unique board wins.

### Approach
- The first 8 moves from the board are random. It generates a valid solution for the board, and chooses a random digit to fill.
- From moves 8 to 16, it's also generating a valid solution. But now, it will search the solution for rectangles to break - and therefore making sure the amount of solutions will go down.
- From move 17, it's possible to create an unique sudoku. The code will take the same approach with the rectangles as above, but will check for each move if the opponent would be able to create an unique solution in the new board. If so, the code will try to find another valid move first.

### A few thoughts
- This was my first project that included bitwise operators. It was kindah fun, and throughout the project, I found more and more places to use them and to speed up some of my code. Absolutely something to keep playing with.
- The code itself is kindah messy and has too much data stored in the global scope. Excecuting some of the functions will update external data used by a lot of functions, creating a hard to follow structure and a source for some nasty bugs.
- It's worth it to take some time to get a close look into the game, to be able to find some game-specefic tactics to implement - something I didn't took time for this year.

15-01-2024