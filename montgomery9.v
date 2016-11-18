`timescale 1ns / 1ps


module montgomery(
    input wire clk,
    input wire resetn,
    input wire start,
    input wire [1023:0] in_a,
    input wire [1023:0] in_b,
    input wire [1023:0] in_m,
    output wire [1023:0] result,
    output wire done
);
     //1024-bit Montgomery Multiplication
     //Add and Sub input size : 1025-bit
     //Add and Sub output size : 1026-bit

     reg [1023 : 0] A ;//A, B and M are 1024-bit
     reg [1023 : 0] M ;
     reg [1025 : 0] C ;//C is at maximum 1026-bit
     wire [1025 : 0] addition_result;


     reg  [1024 : 0] input_A ;
     reg  [1024 : 0] input_B ;

     wire A_i;
     wire C_0 ;
     assign A_i = A[0];
     assign C_0 = C[0];
     assign result = C;

     
     


    parameter [2:0] STATE_IDLE      = 3'b000,
                    STATE_CplusAB   = 3'b001,
                    STATE_C0is0     = 3'b010,
                    STATE_C0is1     = 3'b011,
                    STATE_CbiggerthanM = 3'b100,
                    STATE_keepresult = 3'b101;
    reg  [2:0] state;       //3-bit register
    reg  [2:0] next_state;  //3-bit wire
    reg  [10:0] i_counter;   // count i from 1 to 1024
    reg  [3:0]  addition_counter;
    reg  [6:0] c;
    wire [6:0] cc;
    reg  [2:0] flag = 2'b00;
    assign cc = 7'b00000001;
    wire done_addsub;
    
    reg start_adder;
    reg subtract_adder;
    
    assign done = (state==3'b101) & done_addsub;
    

    // RESET
    always @(posedge clk)
        begin // need signal to reset
            if ( resetn == 1'b0 )begin
                M <= 0;
            end
            else begin
                
                M <= in_m;
             end
        end




    // Sequential Counter logic
    always @(posedge clk)
    begin // need signal to reset
        if ( resetn == 1'b0 )begin
            i_counter <= 0;
            c <= cc;
        end
        else if ((state == STATE_C0is0 && addition_counter == 3'b111)|| (state == STATE_C0is1 && addition_counter == 3'b111)) begin
            i_counter <= i_counter + 1;
            c <= cc;
        end
        else begin
            i_counter <= i_counter;
            c <= (c>>1) ;
         end
    end
   




adder add(.clk(clk), .resetn(resetn), .shift(~c), .in_a(input_A), .in_b(input_B), .start(c), .subtract(subtract_adder), .result(addition_result), .done(done_addsub));



    //ADDER
    always @(posedge clk)
        begin // need signal to reset
        if(resetn == 1'b0) begin
            addition_counter <= 0;
        end
        else if(start_adder == 1'b1) begin
                //start_adder <= 1'b0;
                //adder(clk, resetn, ~start_adder, input_A, input_B, start_adder, subtract_adder, addition_result, done_addsub);
                if(c[0] == 1) addition_counter <= 0;
                else addition_counter <= addition_counter + 1;
            end
        else addition_counter <= 0;
        end

    //F S M
    always@(posedge clk)
    begin
        case (state)

    STATE_IDLE: //00
    begin
	    flag <= 2'b00;
        A <= in_a;
        start_adder <= 1'b1; //next clock to make c[0] == 1 and jump to STATE_CpluseAB
        input_A <= 0;
        subtract_adder <= 1'b0;
        input_B <= 0;
        next_state <= STATE_CplusAB;
        C <= 0;
        end


	STATE_CplusAB:
	begin
	subtract_adder <= 1'b0;

	if(flag == 2'b00) // initialise A, and determine whether to add 0 or B
	    begin
		start_adder <= 1'b0; //next clock to make c[0] == 0 and make shift work
        A <= A >> 1;
		input_A <= C[1024:0];
		if(A_i == 1'b0) begin
			input_B <= 0;
			flag <= 2'b01;
		end
        else begin
			input_B <= in_b;
			flag <= 2'b01;
		end
		C <= C;
		next_state <= STATE_CplusAB;

	    end
	else if(flag == 2'b01) // start start_adder, pass in the signal
	    begin
		input_A <= input_A;
        input_B <= input_B;
        A <= A;
        
        if(done_addsub == 1'b1) begin   //complete the C = C + B 
            C <= addition_result;
            flag <= 2'b00;
            if(C[0] == 1'b0) begin
               start_adder <= 1'b0;
               next_state <= STATE_C0is0;
            end
            else begin
               start_adder <= 1'b1;     //next clock to make c[0] == 1 and jump to STATE_C0is1
               next_state <= STATE_C0is1;
            end
        end
        
        else begin                  //continue the C = C + B until it finish
            start_adder <= 1'b0;
            C <= C;
            flag <= 2'b01;
            next_state <= STATE_CplusAB;
        end    

	end


    STATE_C0is0: //10
    begin
	flag <= 2'b00;
        A <= A;
        start_adder <= 1'b1;    //next clock to make c[0] == 1 and jump to STATE_CplusAB
        input_A <= 0;
        input_B <= 0;
        subtract_adder<=1'b0;
        C <= C >> 1 ;
        next_state <= STATE_CplusAB;
    end


    STATE_C0is1:
    begin
   	subtract_adder <= 1'b0;

    if(flag == 2'b00) // initialise A, and determine whether to add 0 or B
        begin
        start_adder <= 1'b0; //next clock to make c[0] == 0 and make shift work
        A <= A;
        input_A <= C[1024:0];
        input_B <= M;
        flag <= 2'b01;
        C <= C;
        next_state <= STATE_C0is1;
        end
    
    else if(flag == 2'b01) // start start_adder, pass in the signal
        begin
        input_A <= input_A;
        input_B <= input_B;
        A <= A;
        
        if(done_addsub == 1'b1) begin   //complete the C = C + B 
            start_adder <= 1'b1;    //next clock to make c[0] == 1 and jump to STATE_CplusAB
            C <= addition_result;
            flag <= 2'b00;
            next_state <= STATE_CplusAB;
            end
        
        else begin                 //continue the C = C + B until it finish
            start_adder <= 1'b0;    
            C <= C;
            flag <= 2'b01;
            next_state <= STATE_C0is1;
        end    
    end

        



    STATE_CbiggerthanM:
    begin
	flag <= 2'b00;
        A <= A;
        start_adder <= 1'b1;
        input_A <= C[1024:0];
        input_B <= M;
        subtract_adder<=1'b1;
    //adder(clk, resetn, C, M, start, 1'b1, addition_result_CbiggerthanM, done_addsub_CbiggerthanM);
        if(done_addsub == 1'b1) begin
            next_state <= STATE_keepresult;
            C <= addition_result;
        end
        else begin
            next_state <= STATE_CbiggerthanM;
            C <= C;
        end
    end
        
    STATE_keepresult:
    begin
	flag <= 2'b00;
        A <= A;
        start_adder <= 1'b0;
        input_A <= 0;
        input_B <= 0;
        subtract_adder<=1'b0;
        C <= C;
        next_state <= STATE_keepresult;
    end
       
    endcase
    end
    //Synchronous logic to update state
    always @(posedge clk)
    begin
        if (resetn==1'b0)
        begin
            state <= 0;
        end
        else
           state <= next_state;
    end

    /*
    Student tasks:
    1. Instantiate an Adder
    2. Use the Adder to implement the Montgomery multiplier in hardware.
    3. Use tb_montgomery.v to simulate your design.
    */

    //This always block was added to ensure the tool doesn't trim away the montgomery module.
    //Students: Feel free to remove this block
    /*
    reg [1023:0] r_result;
    always @(posedge(clk))
        r_result <= {1024{1'b1}};
    */

endmodule
