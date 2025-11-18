file_path = "gera1.obj"
P = []   # Program Section (Instruções)
M = []   # Data Stack Section (Pilha de dados)
pc = 0   # Program Counter (i)
sp = -1   # Stack Pointer (s)
labels = {}  # Dicionário para mapear labels para endereços

def push_M(value : int):
    global M, sp
    M.append(value)
    sp += 1

def pop_M():
    global M, sp
    M.pop()
    sp -= 1

# Open file 
with open(file_path, 'r') as file:
    lines_list = file.readlines()
    for line in lines_list:
        P.append(line.strip().split())
        print(P[-1])

# First pass: identificar labels
for i, operation in enumerate(P):
    if len(operation) > 1 and operation[1] == "NULL":
        label = operation[0]
        labels[label] = i
        print(f"Label '{label}' encontrado na linha {i}")

print(f"\nLabels mapeados: {labels}\n")

while True:
    operation = P[pc] 
    opcode = operation[0]
    jumped = False
    
    if opcode == "HLT":
        break
    elif opcode == "START":
        sp = -1
    elif opcode == "LDC":
        value = int(operation[1])
        push_M(value)
    elif opcode == "LDV":
        value = int(operation[1])
        push_M(M[value])
    elif opcode == "ADD":
        M[sp - 1] = M[sp - 1] + M[sp]
        pop_M()
    elif opcode == "SUB":
        M[sp - 1] = M[sp - 1] - M[sp]
        pop_M()
    elif opcode == "MULT":
        M[sp - 1] = M[sp - 1] * M[sp]
        pop_M()
    elif opcode == "DIVI":
        M[sp - 1] = M[sp - 1] // M[sp]
        pop_M()
    elif opcode == "INV":
        M[sp] = -M[sp]
    elif opcode == "AND":
        M[sp - 1] = 1 if M[sp - 1] == 1 and M[sp] == 1 else 0
        pop_M()
    elif opcode == "OR":
        M[sp - 1] = 0 if M[sp - 1] == 0 and M[sp] == 0 else 1
        pop_M()
    elif opcode == "NEG":
        M[sp] = 1 - M[sp]
    elif opcode == "CME":
        M[sp - 1] = 1 if M[sp - 1] < M[sp] else 0
        pop_M()
    elif opcode == "CMA":
        M[sp - 1] = 1 if M[sp -1] > M[sp] else 0
        pop_M()
    elif opcode == "CEQ":
        M[sp - 1] = 1 if M[sp - 1] == M[sp] else 0
        pop_M()
    elif opcode == "CDIF":
        M[sp - 1] = 1 if M[sp - 1] != M[sp] else 0
        pop_M()
    elif opcode == "CMEQ":
        M[sp - 1] = 1 if M[sp - 1] <= M[sp] else 0
        pop_M()
    elif opcode == "CMAQ":
        M[sp - 1] = 1 if M[sp -1] >= M[sp] else 0
        pop_M()
    elif opcode == "STR":
        value = int(operation[1])
        M[value] = M[sp]
        pop_M()
    elif opcode == "JMP":
        label = operation[1]
        pc = labels[label]
        jumped = True
    elif opcode == "JMPF":
        label = operation[1]
        if M[sp] == 0: 
            pc = labels[label]
        else: 
            pc += 1
        pop_M()
        jumped = True
    elif len(operation) == 1 and operation[0] not in ["HLT", "START", "INV", "NEG", "RD", "PRN", "RETURN"]:
        pass
    elif len(operation) == 2 and operation[1] == "NULL": 
        pass
    elif opcode == "RD":
        value = (int(input("Entrada: ")))
        push_M(value)
    elif opcode == "PRN":
        print("Saída: ", M[sp])
        pop_M()
    elif opcode == "ALLOC":
        # Alterado: Pega m e n diretamente dos índices (separados por espaço no arquivo)
        m = int(operation[1])
        n = int(operation[2])
        for k in range(n):
            push_M(0)
            M[sp] = M[m + k]
    elif opcode == "DALLOC":
        # Alterado: Pega m e n diretamente dos índices (separados por espaço no arquivo)
        m = int(operation[1])
        n = int(operation[2])
        for k in reversed(range(n)):
            M[m + k] = M[sp]
            pop_M()
    elif opcode == "CALL":
        label = operation[1]
        push_M(pc + 1)
        pc = labels[label]
        jumped = True
    elif opcode == "RETURN":
        pc = M[sp]
        pop_M()
        jumped = True
        
    if not jumped: 
        pc += 1
        
    input("> ")
    print(f"Op = {operation}, PC={pc}, SP={sp}, STACK={[x for x in enumerate(M)]}")