# Processor-Simulator
COL216 Assignment 2

A design decision can be that i don't take any conditional branch and consider it as a normal statement, 
I take unconditional branches as taken ofc, and in this case i flush the IF/ID pipeline
I don't need to stall in that case.

Wherever there is a data hazard, a stall will be introduced before looking for flushes(necessary).

So i can use the original branch_taken signal for the uncond branches and nothing for the conditional branches -> beq,bne,blt,ble,bgt,bge,blz (these will work just fine)

unconditional branches -> jal, jr, jalr ( these will flush the IF/ID pipeline )

how does the flush work? However that works, i have confused it with stall and implemented flush as stall, so i will not change the implementation now.