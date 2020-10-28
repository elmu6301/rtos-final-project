# lab-7-shared-resources-elmu6301
lab-7-shared-resources-elmu6301 created by GitHub Classroom

## Functional Testing
1. At reset observe that no LEDs are lit and the screen displays, speed is *0* and direction is *Straight*. 
2. Press Btn0 and observe that the speed is increased by 5. 
3. Press your finger on the touch slider and slide your finger to the left
	1. Observe that at the far left position, the direction is *Hard Left*
	2. Observe that at the left center position, the direction is *Left*
	3. Observe that at the right center position, the direction is *Right*
	4. Observe that at the far right position, the direction is *Hard Right*
3. Press Btn0 until the speed is 75 and observe that LED0 is lit.
4. Press Btn1 and observe that the speed is decreased by 5. 
5. Press Btn1 until the speed is 45 and and place your finger on the touch slider. 
	1. Observe that LED0 is lit. 
6. Press Btn1 and place your finger on one side of the touch slider. 
	1. Hold for ~5 seconds and observe that LED1 is lit.
	2. Remove your finger from the slider and place on the oposite side. 
	3. Hold for ~5 seconds and observe that LED1 is lit.
	4. Remove your finger from the slider and place on the far left position.
	5. For ~5 seconds move your finger between the far left and left center positions.
	6. Observe that LED1 is lit and that the direction matches your finger's position. 
7. Press Btn1 until the speed is -10 and observe that LED0 is lit. 
    
## System Analysis

Overall, the system is behaving the way I expect it too. For example, when a button is pressed the Setpoint task is called, followed by the Vehicle Monitor task and then the LED output task. This makes sense since the following tasks pend on information from the previous task which is utlimately derived from the Setpoint task. For the Vehicle Direction task, a similar pattern emerges except that this task occurs periodically. Both the LCD and Vehicle Direction tasks are active when their respective timer goes off. 

For screenshots see the _/screenshots_ folder. 
