module processor( input         clk, reset,
                  output [31:0] PC,
                  input  [31:0] instruction,
                  output        WE,
                  output [31:0] address_to_mem,
                  output [31:0] data_to_mem,
                  input  [31:0] data_from_mem
                );
    //... write your code here ...

    reg [31:0] PC_current;
    wire [31:0] PCp;
    wire [31:0] jump_target;
    wire [31:0] branch_target;

    wire [31:0] instr;

    wire [1:0] ALUOp;
    wire [3:0] ALUControl;
    wire [5:0] opcode;
    wire [5:0] funct;
    wire [4:0] shamt;

    wire PCSrcBeq;
    wire RegDst, MemToReg, Branch, MemWrite, ALUSrc, RegWrite, PCSrcJal, PCSrcJr;
    wire CarryOut, Overflow, Zero;

    wire [4:0] write_reg;
    wire [31:0] ALUin1, ALUin2;
    wire [31:0] wd3;
    wire [31:0] to_or_from;
    wire [31:0] signe;
    wire [4:0] a3;

    assign instr = instruction;
    assign PCp = PC + 32'd4;
    assign opcode = instr[31:26];
    assign shamt = instr[10:6];
    assign funct = instr[5:0];
    assign write_reg = (RegDst == 1'b0)? instr[20:16]: (RegDst == 1'b1)? instr[15:11]: 5'b11111;
    assign a3 = PCSrcJal ? 5'b11111 : write_reg;
    assign ALUin2 = ALUSrc ? signe : data_to_mem;
    assign branch_target = (signe << 2) + PCp;
    assign jump_target = {PC[31:28], instr[25:0], 2'b00};
    assign wd3 = PCSrcJal ? PCp : to_or_from;
    assign to_or_from = MemToReg ? data_from_mem : address_to_mem;
    assign PCSrcBeq = Zero && Branch;

    controler control_unit(.Opcode(opcode), .RegDst(RegDst), .MemToReg(MemToReg), .Branch(Branch), .MemWrite(WE), .ALUSrc(ALUSrc), .RegWrite(RegWrite), .PCSrcJal(PCSrcJal), .PCSrcJr(PCSrcJr), .ALUControl(ALUControl), .Funct(funct), .Shamt(shamt));
    regfile32x32 regfile(.CLK(clk), .WE3(RegWrite), .A1(instr[25:21]), .A2(instr[20:16]), .A3(a3), .WD3(wd3), .RD1(ALUin1), .RD2(data_to_mem));
    reg32 register (.in(PC_current), .reset(reset), .clk(clk), .out(PC));
    alu alu1(.A(ALUin1), .B(ALUin2), .ALU_Sel(ALUControl), .ALU_Out(address_to_mem), .CarryOut(CarryOut), .Overflow(Overflow), .Zero(Zero));
    sign_extend se(.in(instr[15:0]), .out(signe));

    always @(*)
      begin
        if(PCSrcBeq)
          PC_current = branch_target;
        else if(PCSrcJr)
          PC_current = ALUin1;
        else if(PCSrcJal)
          PC_current = jump_target;
        else
          PC_current = PCp;
      end
endmodule

module reg32 (input[31:0]in,
                input reset, clk,
                output reg [31:0] out);
        always@(posedge clk)begin
        if(reset)
          out = -4;
        else
          out = in;
        end
endmodule

module controler(input [5:0] Opcode, Funct,
                 input [4:0] Shamt,
                 output reg PCSrcJal, PCSrcJr, RegWrite, MemToReg, MemWrite, ALUSrc, RegDst, Branch,
                 output reg [3:0] ALUControl);

  reg [1:0]ALUOp;

  always @(Opcode, Funct, Shamt)
    begin
      case (Opcode)
        6 'b000000 : begin
          RegWrite = 1;
          RegDst = 1;
          ALUSrc = 0;
          ALUOp = 2 'b10;
          Branch = 0;
          MemWrite = 0;
          MemToReg = 0;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
        6 'b100011 : begin
          RegWrite = 1;
          RegDst = 0;
          ALUSrc = 1;
          ALUOp = 2 'b00;
          Branch = 0;
          MemWrite = 0;
          MemToReg = 1;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
        6 'b101011 : begin
          RegWrite = 0;
          ALUSrc = 1;
          ALUOp = 2 'b00;
          Branch = 0;
          MemWrite = 1;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
        6 'b000100 : begin
          RegWrite = 0;
          ALUSrc = 0;
          ALUOp = 2 'b01;
          Branch = 1;
          MemWrite = 0;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
        6 'b001000 : begin
          RegWrite = 1;
          RegDst = 0;
          ALUSrc = 1;
          ALUOp = 2 'b00;
          Branch = 0;
          MemWrite = 0;
          MemToReg = 0;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
        6 'b000011 : begin
          RegWrite = 1;
          MemWrite = 0;
          PCSrcJal = 1;
          PCSrcJr = 0;
        end
	      6 'b000010 : begin
          RegWrite = 0;
          MemWrite = 0;
          PCSrcJal = 1;
          PCSrcJr = 0;
        end
        6 'b000111 : begin
          RegWrite = 0;
          MemWrite = 0;
          PCSrcJal = 0;
          PCSrcJr = 1;
        end
        6 'b011111 : begin
          RegWrite = 1;
          RegDst = 1;
          ALUSrc = 0;
          ALUOp = 2 'b11;
          Branch = 0;
          MemWrite = 0;
          MemToReg = 0;
          PCSrcJal = 0;
          PCSrcJr = 0;
        end
      endcase
      end

      wire [12:0] in;
      assign in = {ALUOp, Funct, Shamt};
      always @(in)
       casex (in)
         13'b00xxxxxxxxxxx: ALUControl = 4'b0010;
         13'b01xxxxxxxxxxx: ALUControl = 4'b0110;
         13'b10100000xxxxx: ALUControl = 4'b0010;
         13'b10100010xxxxx: ALUControl = 4'b0110;
         13'b10100100xxxxx: ALUControl = 4'b0000;
         13'b10100101xxxxx: ALUControl = 4'b0001;
         13'b10101010xxxxx: ALUControl = 4'b0111;
         13'b10000100xxxxx: ALUControl = 4'b1111;
         13'b1000011000000: ALUControl = 4'b1110;
         13'b1000011100000: ALUControl = 4'b1100;
         13'b1101000000000: ALUControl = 4'b1000;
         13'b1101000000100: ALUControl = 4'b1001;
       endcase
endmodule

module regfile32x32 (input [4:0] A1,A2,A3,
                 input [31:0] WD3,
                 input WE3, CLK,
                 output[31:0] RD1,RD2);
        reg [31:0]rf[31:0];
               assign RD1 = rf[A1];
               assign RD2 = rf[A2];
        always@(posedge CLK)
        begin
                if (WE3)
                begin
                        rf[A3]=WD3;
                end
                rf[0]=0;
        end
endmodule

module sign_extend(
  input [15:0] in,
  output [31:0] out
);

assign out = {{16{in[15]}}, in};

endmodule


module alu(input [31:0] A, B,
           input [3:0] ALU_Sel,
           output reg Zero, CarryOut, Overflow,
           output reg [31:0] ALU_Out);

           function [7:0] satsum(input [7:0] A,B);
           begin
             CarryOut = 0;
             {CarryOut,satsum[7:0]} = A[7:0] + B[7:0];
             if (CarryOut)
               satsum[7:0] = 'b11111111;
             CarryOut = 0;
           end
           endfunction

           function [31:0] sum(input [31:0] A,B);
           begin
           {CarryOut,sum} = A + B;
           if (sum [31] == 0 && A [31] == 1 && B [31] == 1 )
                   Overflow = 1;
           if (sum [31] == 1 && A [31] == 0 && B [31] == 0 )
                   Overflow = 1;
           end
           endfunction

           function [31:0] deduct(input [31:0] A,B);
           begin
           deduct = A - B;
           if (deduct [31] == 0 && A [31] == 1 && ~B [31] == 1 )
                   Overflow = 1;
           if (deduct [31] == 1 && A [31] == 0 && ~B [31] == 0 )
                   Overflow = 1;
           end
           endfunction

        always @(A, B, ALU_Sel)
         begin
          Zero = 0;
          CarryOut = 0;
          Overflow = 0;
          case(ALU_Sel)
            4'b0010:  begin                               // add
                ALU_Out = sum(A[31:0],B[31:0]);
                end
            4'b0110:  begin                               // sub
                ALU_Out = deduct(A[31:0],B[31:0]);
                end
            4'b0000:  begin
                ALU_Out = A & B;                          // and
                end
            4'b0001:  begin
                ALU_Out = A | B;                          // or
                end
            4'b0111:  begin
                ALU_Out = $signed(A) < $signed(B);        // slt
                end
            4'b0011:  begin                               // xor
                ALU_Out = A ^ B;
                end
            4'b1000:  begin                               // byte adds
                ALU_Out[7:0] = A[7:0] + B[7:0];
                ALU_Out[15:8] = A[15:8] + B[15:8];
                ALU_Out[23:16] = A[23:16] + B[23:16];
                ALU_Out[31:24] = A[31:24] + B[31:24];
                end
            4'b1001:  begin                              // satsum adds
                ALU_Out[7:0] = satsum(A[7:0],B[7:0]);
                ALU_Out[15:8] = satsum(A[15:8],B[15:8]);
                ALU_Out[23:16] = satsum(A[23:16],B[23:16]);
                ALU_Out[31:24] = satsum(A[31:24],B[31:24]);
                end
	          4'b1111:  begin
                ALU_Out = B << A;
                end
            4'b1110:  begin
                ALU_Out = $unsigned(B) >>> A;
                end
            4'b1100:  begin
                ALU_Out = $signed(B) >>> A;
                end
          endcase

          if(ALU_Out == 0)
            Zero = 1;
        end
endmodule
