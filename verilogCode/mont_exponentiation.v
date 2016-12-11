`timescale 1ns / 1ps
module montgomery_exp(
    input wire clk,
    input wire resetn,
    input wire start,
    input wire [1023:0] R2_modm,
    input wire [1023:0] R_modm,
    input wire [1023:0] in_m,
    input wire [1023:0] message,
    input wire [1023:0] d,
    output wire [1023:0] result,
    output wire done
);
    assign result = A;
    assign done = (state == STATE_keepresult);
    
    reg start_mont;
    reg  [1023 : 0] input_A ;
    reg  [1023 : 0] input_B ;
    reg  [1023 : 0] input_M ;
    wire [1023 : 0] mont_result;
    wire done_mont;

 montgomery mont(.clk(clk), .resetn(resetn), .start(start_mont), .in_a(input_A), .in_b(input_B), .in_m(input_M), .result(mont_result), .done(done_mont));
    
    parameter [2:0]  STATE_IDLE       = 3'b000,
                     STATE_init       = 3'b001,
                     STATE_loop         = 3'b010,
                     STATE_loop_out         = 3'b011,
                     STATE_return          = 3'b100,
                     STATE_keepresult = 3'b101;
     reg  [2:0] state;       //3-bit register
     reg  [2:0] next_state;  //3-bit wire
     reg  [2:0] prev_state;
     
     reg  [1023 : 0] x_tilda;
     reg  [1023 : 0] A;
     reg  [10 : 0] i_count;
     reg flag_loop;
          
          
     always@(posedge clk) begin
        if(state == STATE_IDLE)
            i_count <= 0;
        else if( flag_loop==1'b1 || (next_state == STATE_loop && state == STATE_loop_out))begin
                 i_count <= i_count + 1;            
            end
            else i_count <= i_count;
     end
     
        //F S M
     always@(*)
     begin
         case (state)
         STATE_IDLE:
         begin
             start_mont <= 1'b0;
             input_A <= 0;
             input_B <= 0;
             input_M <= 0;
             next_state <= STATE_init;
             x_tilda <= 0;
             A <= 0;
             flag_loop <= 1'b0;
         end
 
         STATE_init:          
         begin
             input_A <= message;
             input_B <= R2_modm;
             input_M <= in_m;
             A <= R_modm;
             flag_loop <= 1'b0;
             if(prev_state != STATE_init)    start_mont <= 1'b1;
             else start_mont <= 1'b0;
             
             if(done_mont == 1'b1)begin
                next_state <=  STATE_loop;
                x_tilda <= mont_result;
             end 
             else begin 
                next_state <=  STATE_init;
                x_tilda <= 0;
             end
         end
         
         
 
         STATE_loop:
         begin
             input_A <= A;
             input_B <= A;
             input_M <= in_m;
             x_tilda <= x_tilda;
             
             if(i_count[10] == 1'b1)begin
                next_state <=  STATE_return;
                start_mont <= 1'b0;
                flag_loop <= 1'b0;
                A <= A;
             end
             else begin
                if(prev_state != STATE_init || flag_loop == 1'b1) start_mont <= 1'b1;
                else start_mont <= 1'b0;

                if(done_mont == 1'b1)begin
                    if(d[i_count] == 1'b0)begin
                        next_state <=  STATE_loop;
                        flag_loop <= 1'b1;
                    end
                    else begin
                        next_state <=  STATE_loop_out;
                        flag_loop <= 1'b0;
                    end
                    A <= mont_result;
                end 
                else begin 
                    next_state <=  STATE_loop;
                    A <= A;
                    flag_loop <= 1'b0;
                end
             end
         end
 
         STATE_loop_out:
         begin
             input_A <= A;
             input_B <= x_tilda;
             input_M <= in_m;
             x_tilda <= x_tilda;
             flag_loop <= 1'b0;
             
             if(prev_state != STATE_loop_out) start_mont <= 1'b1;
             else start_mont <= 1'b0;
             
            
            if(done_mont == 1'b1)begin
                 next_state <= STATE_loop;
                 A <= mont_result;
             end
             else begin
                 next_state <= STATE_loop_out;
                 A <= A;
             end
         end
 
         STATE_return:
         begin
             input_A <= A;
             input_B <= 1'b1;
             input_M <= in_m;
             x_tilda <= x_tilda;
             flag_loop <= 1'b0;
             
             if(prev_state != STATE_return)    start_mont <= 1'b1;
             else start_mont <= 1'b0;
             
             if(done_mont == 1'b1) begin
                 next_state <= STATE_keepresult;
                 A <= mont_result;       
             end
             else begin
                 next_state <= STATE_return;
                 A <= A;
             end
         end
         
         STATE_keepresult:
         begin
             start_mont <= 1'b0;
             input_A <= 0;
             input_B <= 0;
             input_M <= 0;
             next_state <= STATE_keepresult;
             x_tilda <= x_tilda;
             A <= A;
             flag_loop <= 1'b0;
         end        
     endcase
   end



        //Synchronous logic to update state
    always @(posedge clk)
    begin
        if (start == 1'b1)
        begin
            prev_state <= 0;
            state <= 0;    
        end
        else begin
             prev_state <= state;
             state <= next_state;            
        end

    end
endmodule

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

     //reg [1023 : 0] A ;//A, B and M are 1024-bit
     reg [1025 : 0] C ;//C is at maximum 1026-bit
     wire [1025 : 0] addition_result;


     reg  [1024 : 0] input_A ;
     reg  [1024 : 0] input_B ;

     wire A_i;
     wire C_0 ;
     //assign A_i = A[0];
     assign C_0 = C[0];
     assign result = C[1023:0];


    parameter [2:0] STATE_IDLE      = 3'b000,
                    STATE_CplusAB   = 3'b001,
                    STATE_C0is0     = 3'b010,
                    STATE_C0is1     = 3'b011,
                    STATE_CbiggerthanM = 3'b100,
                    STATE_keepresult = 3'b101;
    reg  [2:0] state;       //3-bit register
    reg  [2:0] next_state;  //3-bit wire
    reg  [2:0] prev_state;
    reg  [10:0] i_counter;   // count i from 1 to 1024
    assign A_i = in_a[i_counter];
    
    reg  [2:0]  addition_counter;
    reg  [6:0] c;
    wire [6:0] cc;
    assign cc = 6'b0000001;
    wire done_addsub;
    
    reg start_adder;
    reg subtract_adder;
    
    assign done = (state==3'b101);
    assign result = C; 
    
    /*     
     // Sequential Counter logic
     always @(posedge clk)
        begin // need signal to reset
            if ( resetn == 1'b0 )begin
               c <= 0;
            end
            else begin
                if(start_adder == 1'b1) c <= cc;
                else c <= (c>>1) ;
            end
        end
*/
    adder add(.clk(clk), .resetn(resetn), .shift(~start_adder), .in_a(input_A), .in_b(input_B), .start(start_adder), .subtract(subtract_adder), .result(addition_result), .done(done_addsub));
       
    //ADDER
    /*always @(posedge clk)
    begin // need signal to reset
    if(resetn == 1'b0) begin
        addition_counter <= 0;
    end
    else if(start_adder == 1'b1) begin
            if(c[0] == 1) addition_counter <= 0;
            else addition_counter <= addition_counter + 1;
        end
    else addition_counter <= addition_counter;
    end*/
    
    always@(posedge clk) begin
        if( (state == STATE_CplusAB && A_i == 1'b1 && done_addsub == 1'b1) || (state==STATE_CbiggerthanM && done_addsub == 1'b1 && addition_result[1025] == 1'b0))begin
            C <= addition_result;            
        end
        else if( (state == STATE_C0is1 && done_addsub == 1'b1) )begin
            C <= addition_result[1025:1] ;
        end
        else if(state == STATE_C0is0)begin
            C <= C >> 1;
        end
        else if(state == STATE_IDLE) C <= 0;
        else C <= C;
    end
    
     
    always@(posedge clk) begin
        if(state == STATE_IDLE)
             i_counter <= 0;
        else if( (next_state == STATE_CplusAB && state == STATE_C0is0) || (next_state == STATE_CplusAB && state == STATE_C0is1))begin
             i_counter <= i_counter + 1;            
        end
        else i_counter <= i_counter;
    end
    
    //F S M
    always@(*)
    begin
        case (state)

        STATE_IDLE:
        begin
            
            start_adder <= 1'b0;
            input_A <= 0;
            subtract_adder <= 1'b0;
            input_B <= 0;
            next_state <= STATE_CplusAB;
        end

        STATE_CplusAB:          
        begin
            subtract_adder<=1'b0;
            input_A <= C[1024:0];
            
            if(prev_state != STATE_CplusAB)    start_adder <= 1'b1;
            else start_adder <= 1'b0;
            
            if(A_i == 1'b0 && C[0] == 1'b0) begin
                input_B <= 0;
                //if(C[0] == 1'b0)begin
                next_state <= STATE_C0is0;
                   
            end
            else if(A_i == 1'b0 && C[0] == 1'b1) begin
                next_state <= STATE_C0is1;
                input_B <= 0;  
            end
            else if(A_i == 1'b1 && done_addsub == 1'b1 && addition_result[0] == 1'b0) begin
                input_B <= in_b;
                next_state <= STATE_C0is0; 
            end
            else if(A_i == 1'b1 && done_addsub == 1'b1 && addition_result[0] == 1'b1) begin
                input_B <= in_b;
                next_state <= STATE_C0is1; 
            end
            else begin 
                input_B <= in_b;
                next_state <= STATE_CplusAB;
            end
                /*if(done_addsub == 1'b1 && addition_result[0] == 1'b0) begin
                     next_state <= STATE_C0is0;                   
                end
                else if(done_addsub == 1'b1 && addition_result[0] == 1'b1) begin
                     next_state <= STATE_C0is1;                    
                end
                else begin 
                    next_state <= STATE_CplusAB;
                end*/
        end
        
        

        STATE_C0is0:
        begin
            subtract_adder<=1'b0;
            start_adder <= 1'b0;
            input_A <= 0;
            input_B <= 0;
    
            if(i_counter == 10'b1111111111) next_state <= STATE_CbiggerthanM;
            else next_state <= STATE_CplusAB;
            
        end

        STATE_C0is1:
        begin
            subtract_adder<=1'b0;
            input_A <= C[1024:0];
            input_B <= in_m;
            
            if(prev_state != STATE_C0is1)    start_adder <= 1'b1;
            else start_adder <= 1'b0;
            
           
            if(done_addsub == 1'b1 && i_counter == 10'b1111111111) 
            begin
               next_state <= STATE_CbiggerthanM; 
            end
            else if(done_addsub == 1'b1)
            begin
                next_state <= STATE_CplusAB;
               
            end
            else begin
                next_state <= STATE_C0is1;
              
            end
        end

        STATE_CbiggerthanM:
        begin
            input_A <= C[1024:0];
            input_B <= in_m;
            subtract_adder<=1'b1;
            
            if(prev_state != STATE_CbiggerthanM)    start_adder <= 1'b1;
            else start_adder <= 1'b0;
            
            if(done_addsub == 1'b1) begin
                next_state <= STATE_keepresult;              
            end
            else begin
                next_state <= STATE_CbiggerthanM;
            end
        end
        
        STATE_keepresult:
        begin
            start_adder <= 1'b0;
            input_A <= 0;
            input_B <= 0;
            subtract_adder<=1'b0;
            next_state <= STATE_keepresult;
        end
       
    endcase
    end
    
    //Synchronous logic to update state
    always @(posedge clk)
    begin
        if (start == 1'b1)
        begin
            prev_state <= 0;
            state <= 0;    
        end
        else begin
             prev_state <= state;
             state <= next_state;            
        end

    end
    /*
    always@(posedge clk) begin
        if(state == STATE_IDLE || (state == STATE_CplusAB && prev_state == STATE_IDLE)) begin
             //i_counter <=0;
             A <= in_a;
        end
        else if( (next_state == STATE_CplusAB && state == STATE_C0is0) || (next_state == STATE_CplusAB && state == STATE_C0is1))begin
             A <= A >> 1;
             //i_counter <= i_counter +1;            
        end
        else begin
            A <= A;
            //i_counter <= i_counter;
        end
    end
    */
    
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