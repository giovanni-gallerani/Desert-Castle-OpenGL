Computer Graphics Exam - 2021-2022  
Teacher: Antonino Casile  
Student: GIOVANNI GALLERANI  
---  
**Note: the comments in the main.c file are not fully translated in English since this is an old project of mine.**  
  
This project presents the model of a castle inspired by the aesthetics of the Nintendo 64 game "The Legend of Zelda: Ocarina of Time".  
  
The castle initially protrudes from the sand, holding down the 'space' key an animation makes the castle emerge from under the sand.  
First the walls emerge and when these are almost completely raised the castle inside them also begins to rise.  
During this animation the scene rotates.  
  
At the end of the animation it is possible to raise and lower the drawbridge at will by pressing 'u' (up) and 'd' (down).  
  
Pressing 'f' enables/disables the free rotation mode of the scene (disabled by default), this mode was created with the aim of observing how the model was created since the animation and the model are based on a perspective game, it is so to be understood as a sort of debugging mode.  
In free rotation mode press 'x', 'X', 'y', 'Y', 'z', 'Z' to rotate the scene.  
  
Pressing 'r' resets the animation and disables the free rotation setting of the scene.  
  
Pressing 'esc' terminate the program.  
  
To compile the code run on the terminal the following command
```
gcc -o main main.c shader.c readBMP.c cylinder.c rectangle.c triangle.c square.c shader.h readBMP.h cylinder.h rectangle.h triangle.h square.h vertex.h light.h material.h -lm -lGL -lX11 -lGLEW -lglut -lGLU -lc
```