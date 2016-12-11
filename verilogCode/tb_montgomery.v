`timescale 1ns / 1ps
`define RESET_TIME 15
`define CLK_PERIOD 10
`define CLK_HALF 5

module tb_montgomery(   
    );
    
    reg clk,resetn;
    reg [1023:0] R2_modm, R_modm, in_m,d,message;
    reg start;
    wire [1023:0] result;
    wire done;
    
    //Instantiating montgomery module
    /*montgomery montgomery_instance
           (.clk(clk),
            .resetn(resetn),
            .in_a(in_a),
            .in_b(in_b),
            .in_m(in_m),
            .start(start),
            .result(result),
            .done(done)
            );*/
    montgomery_exp montgomery_instance
                       (.clk(clk),
                        .resetn(resetn),
                        .start(start),
                        .R2_modm(R2_modm),
                        .R_modm(R_modm),
                        .in_m(in_m),
                        .message(message),
                        .d(d),
                        .result(result),
                        .done(done)
                        );
    //Generate a clock
    initial begin
        clk = 0;
        forever #`CLK_HALF clk = ~clk;
    end
    
    //Reset
    initial begin
        resetn = 0;
        #`RESET_TIME resetn = 1;
    end
    
    //Test data
    initial begin
        #`RESET_TIME
        
        //First test vector:
        R_modm<=1024'h46207C75B2B76361CD2BA2F2FEC47E26EFB7703552E663E7CC008AF004D15F36312A66F29512612548A877608E7E8EDC82663F42CB862EA2B6F023D73784676F8772503E4A9FB8EE0AE928DA197215538E26A6E8D1490D32E68489ECAFA45D9A4C539452F93E336B216A08BE9810C1334E7BB00B43F30EEF14DB4E7BB6881C1A;
        R2_modm<=1024'h187C74C74D55B6F97A8F5F54900034AAD155419F0514032AC0F25644D2748F7E9E7DC826CF649200B9D3F3E44FB6921206962D2B087E2AE020D5C5A12474E0166A18D5B2009458F406116E78B18A35B703F734A58D427386D3700427FD89D50854D4A876F16540440CD5908AEE3860140B60638EC738C287FAF6E1B18CF805FD;
        in_m<=1024'hD1D3B4D60ED3982C36A53DD9CFB0450E6887926D199AF8FEE7990185F907210129AB96B24F2E543827028A894DA058EC2DAAE084358E7C2456BA1EB0CF1AE468093C99331D501EA97F89EBB5E1709725DC771F4293ADE44605453C47716A5C3B8E88E8CF2ADE1186BE08BC1E2AB9A7C832DFF4023B9E66F8A677BCE5E67A6F31;
        d<=1024'h397F69EF837C90130DBB37ADDC05F4DE0A8404A2CDF380790B1ABA20562AD01E8D2E26E0C9EADFF4F6F8B6374C733F63225EE07A4784503443EC805E4BF66B74A010915320FF91B8C6EAAA9AA88256A3A11DE2FF803B6C49412F0B0CD973EAE6687629F7C78F65491FD8473BB8731DA07F7156D678800FC92B9DC874E8C0D201;
        message<=1024'h815CB73D03DE810F550BD84165A3D1AD7E014E21E005A53BEC3335BE781EE92B54763211EE5CBA8900362269FB4ECFC977E9E017C8534815414383E2D29831D2FDB408687C4B38E9A5DF01533C43A4E2CF0A6D3B0233F1D44A415B015D274B56E3D9096F0AC06D97C662A0019384A9E0FBD369C4776E77D3F4BD35B6395C695;
        start<=1;
        #`CLK_PERIOD;
        
        start<=0;
        wait (done==1);
        $display("result=%x",result);
        #`CLK_PERIOD;
        
        //Second test vector:
        //The test vector that was given in mtgo1024.c on Toledo
        //in_a<=1024'h1BA;
        //in_b<=1024'h91B;
        //in_m<=1024'hD1D3B4D60ED3982C36A53DD9CFB0450E6887926D199AF8FEE7990185F907210129AB96B24F2E543827028A894DA058EC2DAAE084358E7C2456BA1EB0CF1AE468093C99331D501EA97F89EBB5E1709725DC771F4293ADE44605453C47716A5C3B8E88E8CF2ADE1186BE08BC1E2AB9A7C832DFF4023B9E66F8A677BCE5E67A6F31;
        //start<=1;
        //#`CLK_PERIOD;
        //start<=0;
        //wait (done==1);
        //$display("result=%x",result);
        //#`CLK_PERIOD;
    end
           
endmodule
