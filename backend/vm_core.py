# backend/vm_core.py
# Máquina Virtual Didática (MVD)
#
# Regras principais implementadas:
# - Labels numéricos/textuais reconhecidos quando aparecem no início da linha:
#     Ex: "1 NULL", "2: NULL", "L1 NULL", "LOOP: NULL"
# - NÃO substituir tokens numéricos usados como argumentos de memória
#   (ex: ALLOC 0 1) — para evitar ALLOC 0 1 -> ALLOC 0 <endereço do label 1>
# - Substituição de labels por endereços ocorre apenas para instruções de
#   salto/chamada: CALL, JMP, JMPF (argumentos de controle de fluxo)
# - CALL empilha endereço de retorno na pilha de dados (M[++s] = pc+1)
# - RETURN desempilha endereço de retorno da pilha de dados
# - RD exige input (lança VMError se fila vazia); enqueue_input "acorda" a VM

class VMError(Exception):
    pass


class VM:
    def __init__(self):
        self.reset_all()

    def reset_all(self):
        # programa / montagem
        self.P = []            # lista de instruções tokenizadas (cada instrução -> list[str])
        self.labels = {}       # mapa label -> endereço (índice em P)

        # memória / pilha de dados
        self.M = {}            # memória (endereço:int -> valor:int)
        self.s = -1            # topo da pilha (índice)

        # controle de execução
        self.pc = 0
        self.halted = False
        self.last_error = None
        self.output = []

        # I/O
        self.input_queue = []

    # -----------------------
    # Montador / Carregador
    # -----------------------
    def load_program(self, asm_text):
        """
        Montador em duas passagens.
        - Detecta labels quando aparecem no início da linha (numéricos ou textuais),
          aceitando espaços/tabs variados e opcional ':'.
        - Substitui labels por endereços SOMENTE para argumentos de controle de fluxo:
          CALL, JMP, JMPF.
        """
        # reset parcial (mantém instância)
        self.P = []
        self.labels = {}
        self.M = {}
        self.s = -1
        self.pc = 0
        self.halted = False
        self.last_error = None
        self.output = []
        self.input_queue = []

        valid_instr = {
            'START', 'LDC', 'LDV', 'ADD', 'SUB', 'MULT', 'DIVI', 'INV',
            'AND', 'OR', 'NEG', 'CME', 'CMA', 'CEQ', 'CDIF', 'CMEQ', 'CMAQ',
            'STR', 'JMP', 'JMPF', 'NULL', 'RD', 'PRN',
            'ALLOC', 'DALLOC',
            'CALL', 'RETURN', 'HLT'
        }

        raw_lines = asm_text.splitlines()

        # --- 1ª passagem: registrar labels (apenas se aparecerem no início da linha) ---
        for raw in raw_lines:
            if raw is None:
                continue
            line = raw.strip()
            if not line:
                continue

            parts = line.split()
            if not parts:
                continue

            first_token = parts[0]

            # normalize: remover eventual ':' do final
            t_clean = first_token.rstrip(':')

            # Se first_token é um número puro, consideramos label numérico
            # (apenas se estiver no início da linha)
            if t_clean.isdigit():
                label = t_clean
                # registra label apontando para próximo endereço em P
                self.labels[label] = len(self.P)
                # criar alias 'L<num>' para conveniência (ex: 'L1' -> '1' e vice-versa)
                l_alias = 'L' + label
                if l_alias not in self.labels:
                    self.labels[l_alias] = self.labels[label]
                # se houver instrução após o número, anexa
                if len(parts) > 1:
                    self.P.append(parts[1:])
                # caso contrário (linha só com "N" sem instrução) => nada a adicionar
                continue

            # Se token termina com ":" (ex: "L1:") -> label textual
            if first_token.endswith(':'):
                label = t_clean
                self.labels[label] = len(self.P)
                # criar alias numérico se label for L<num>
                if label.upper().startswith('L') and label[1:].isdigit():
                    num = label[1:]
                    if num not in self.labels:
                        self.labels[num] = self.labels[label]
                # se restante existe, anexa como instrução
                if len(parts) > 1:
                    self.P.append(parts[1:])
                else:
                    # se só "LABEL:" sem instrução, tratamos como NULL
                    self.P.append(['NULL'])
                continue

            # caso: linha começa com instrução (first token é instr válida)
            if t_clean.upper() in valid_instr:
                self.P.append(parts)
                continue

            # caso: linha começa com palavra não-instrucao (p.ex. 'L1 NULL' sem ':' ou 'LABEL NULL')
            # se houver "NULL" logo após, tratamos isso como label também
            if len(parts) >= 2 and parts[1].upper() == 'NULL':
                label = t_clean
                self.labels[label] = len(self.P)
                # alias 'L<num>' se for L#
                if label.upper().startswith('L') and label[1:].isdigit():
                    num = label[1:]
                    if num not in self.labels:
                        self.labels[num] = self.labels[label]
                if label.isdigit():
                    lalias = 'L' + label
                    if lalias not in self.labels:
                        self.labels[lalias] = self.labels[label]
                # append NULL instruction
                self.P.append(['NULL'])
                continue

            # caso geral: assumimos que a linha é 'LABEL instr...' sem ':' (por segurança)
            # registra label e anexa o que resta
            if t_clean not in valid_instr:
                label = t_clean
                self.labels[label] = len(self.P)
                if label.upper().startswith('L') and label[1:].isdigit():
                    num = label[1:]
                    if num not in self.labels:
                        self.labels[num] = self.labels[label]
                if len(parts) > 1:
                    self.P.append(parts[1:])
                else:
                    self.P.append(['NULL'])
                continue

            # fallback — tratar como instrução
            self.P.append(parts)

        # --- 2ª passagem: substituir labels por endereços, MAS somente para ops de fluxo de controle ---
        for i, instr in enumerate(self.P):
            if not instr:
                continue
            op = str(instr[0]).upper()

            # Apenas CALL, JMP, JMPF têm seu argumento 1 substituído por endereço de label, quando aplicável
            if op in ('CALL', 'JMP', 'JMPF'):
                # se houver argumento
                if len(instr) > 1:
                    tok = str(instr[1]).rstrip(':')
                    if tok in self.labels:
                        instr[1] = str(self.labels[tok])
                # guarda de volta
                self.P[i] = instr
            else:
                # para outras instruções, não alteramos tokens (mantemos literais numéricos)
                self.P[i] = instr

    # -----------------------
    # Reiniciar execução (mantém programa carregado)
    # -----------------------
    def reset(self):
        self.M = {}
        self.s = -1
        self.pc = 0
        self.halted = False
        self.last_error = None
        self.output = []
        self.input_queue = []

    # -----------------------
    # Execução: step
    # -----------------------
    def step(self):
        if self.halted or self.pc < 0 or self.pc >= len(self.P):
            self.halted = True
            return

        try:
            instr = self.P[self.pc]
            if not instr:
                self.pc += 1
                return

            op = str(instr[0]).upper()

            def parse_arg(idx):
                if len(instr) <= idx:
                    return None
                tok = instr[idx]
                ts = str(tok)
                # se já for representação numérica (string de dígitos, possivelmente negativo)
                if ts.lstrip('-').isdigit():
                    return int(ts)
                return tok

            arg1 = parse_arg(1)
            arg2 = parse_arg(2)

            # --- instruções ---
            if op == "START":
                self.s = -1
                self.pc += 1
                return

            if op == "LDC":
                self.s += 1
                self.M[self.s] = int(arg1)
                self.pc += 1
                return

            if op == "LDV":
                self.s += 1
                self.M[self.s] = self.M.get(int(arg1), 0)
                self.pc += 1
                return

            if op == "ADD":
                if self.s < 1:
                    raise VMError("ADD: pilha insuficiente")
                self.M[self.s - 1] = self.M.get(self.s - 1, 0) + self.M.get(self.s, 0)
                self.s -= 1
                self.pc += 1
                return

            if op == "SUB":
                if self.s < 1:
                    raise VMError("SUB: pilha insuficiente")
                self.M[self.s - 1] = self.M.get(self.s - 1, 0) - self.M.get(self.s, 0)
                self.s -= 1
                self.pc += 1
                return

            if op == "MULT":
                if self.s < 1:
                    raise VMError("MULT: pilha insuficiente")
                self.M[self.s - 1] = self.M.get(self.s - 1, 0) * self.M.get(self.s, 0)
                self.s -= 1
                self.pc += 1
                return

            if op == "DIVI":
                if self.s < 1:
                    raise VMError("DIVI: pilha insuficiente")
                if self.M.get(self.s, 0) == 0:
                    raise VMError("Divisão por zero")
                self.M[self.s - 1] = self.M.get(self.s - 1, 0) // self.M.get(self.s, 0)
                self.s -= 1
                self.pc += 1
                return

            if op == "INV":
                if self.s < 0:
                    raise VMError("INV: pilha vazia")
                self.M[self.s] = -self.M.get(self.s, 0)
                self.pc += 1
                return

            if op == "AND":
                if self.s < 1:
                    raise VMError("AND: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) == 1 and self.M.get(self.s, 0) == 1) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "OR":
                if self.s < 1:
                    raise VMError("OR: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) == 1 or self.M.get(self.s, 0) == 1) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "NEG":
                if self.s < 0:
                    raise VMError("NEG: pilha vazia")
                self.M[self.s] = 1 - self.M.get(self.s, 0)
                self.pc += 1
                return

            if op == "CME":
                if self.s < 1:
                    raise VMError("CME: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) < self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "CMA":
                if self.s < 1:
                    raise VMError("CMA: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) > self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "CEQ":
                if self.s < 1:
                    raise VMError("CEQ: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) == self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "CDIF":
                if self.s < 1:
                    raise VMError("CDIF: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) != self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "CMEQ":
                if self.s < 1:
                    raise VMError("CMEQ: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) <= self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "CMAQ":
                if self.s < 1:
                    raise VMError("CMAQ: pilha insuficiente")
                self.M[self.s - 1] = 1 if (self.M.get(self.s - 1, 0) >= self.M.get(self.s, 0)) else 0
                self.s -= 1
                self.pc += 1
                return

            if op == "STR":
                if self.s < 0:
                    raise VMError("STR: pilha vazia")
                if arg1 is None:
                    raise VMError("STR: argumento ausente")
                addr = int(arg1)
                self.M[addr] = self.M.get(self.s, 0)
                self.s -= 1
                self.pc += 1
                return

            if op == "JMP":
                if arg1 is None:
                    raise VMError("JMP: argumento ausente")
                self.pc = int(arg1)
                return

            if op == "JMPF":
                if self.s < 0:
                    raise VMError("JMPF: pilha vazia")
                cond = self.M.get(self.s, 0)
                self.s -= 1
                if cond == 0:
                    if arg1 is None:
                        raise VMError("JMPF: argumento ausente")
                    self.pc = int(arg1)
                else:
                    self.pc += 1
                return

            if op == "NULL":
                self.pc += 1
                return

            if op == "RD":
                self.s += 1
                if self.input_queue:
                    self.M[self.s] = int(self.input_queue.pop(0))
                    self.pc += 1
                else:
                    raise VMError("RD attempted but input queue empty")
                return

            if op == "PRN":
                if self.s < 0:
                    raise VMError("PRN: pilha vazia")
                self.output.append(self.M.get(self.s, 0))
                self.s -= 1
                self.pc += 1
                return

            if op == "ALLOC":
                if arg1 is None or arg2 is None:
                    raise VMError("ALLOC: argumentos ausentes")
                m = int(arg1)
                n = int(arg2)
                for k in range(n):
                    self.s += 1
                    self.M[self.s] = self.M.get(m + k, 0)
                self.pc += 1
                return

            if op == "DALLOC":
                if arg1 is None or arg2 is None:
                    raise VMError("DALLOC: argumentos ausentes")
                m = int(arg1)
                n = int(arg2)
                if self.s < n - 1:
                    raise VMError("DALLOC: pilha insuficiente")
                for k in range(n - 1, -1, -1):
                    self.M[m + k] = self.M.get(self.s, 0)
                    self.s -= 1
                self.pc += 1
                return

            if op == "CALL":
                if arg1 is None:
                    raise VMError("CALL: argumento ausente")
                self.s += 1
                self.M[self.s] = self.pc + 1
                self.pc = int(arg1)
                return

            if op == "RETURN":
                if self.s < 0:
                    raise VMError("RETURN: pilha vazia")
                self.pc = int(self.M.get(self.s, 0))
                self.s -= 1
                return

            if op == "HLT":
                self.halted = True
                return

            raise VMError(f"Instrução inválida: {op}")

        except VMError as e:
            self.last_error = str(e)
            if "RD attempted" in str(e):
                self.halted = False
            else:
                self.halted = True
            raise
        except Exception as e:
            self.halted = True
            self.last_error = f"Erro inesperado: {e}"
            raise VMError(self.last_error)

    # -----------------------
    # run / utilitários
    # -----------------------
    def run(self, step_limit=1000000):
        count = 0
        while not self.halted and count < step_limit:
            self.step()
            count += 1
        if count >= step_limit:
            raise VMError("Limite de passos atingido")

    def enqueue_input(self, value):
        try:
            self.input_queue.append(int(value))
        except Exception:
            raise VMError("enqueue_input: valor inválido")
        if self.last_error and "RD attempted" in str(self.last_error):
            # limpa erro para permitir continuação
            self.last_error = None

    def snapshot(self):
        return {
            "pc": self.pc,
            "stack": [self.M.get(i, 0) for i in range(self.s + 1)],
            "mem": {k: v for k, v in sorted(self.M.items())},
            "output": self.output.copy(),
            "halted": self.halted,
            "last_error": self.last_error,
            "next_instr": ' '.join(map(str, self.P[self.pc])) if 0 <= self.pc < len(self.P) else None
        }

    def dump_program(self):
        labels_rev = {v: k for k, v in self.labels.items()}
        lines = []
        for i, instr in enumerate(self.P):
            lbl = labels_rev.get(i, "")
            label_str = (lbl + ":") if lbl else ""
            lines.append(f"{i:03d} {label_str}\t{' '.join(map(str, instr))}")
        return "\n".join(lines)
