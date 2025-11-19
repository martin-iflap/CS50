from typing import List

class Candidate:
    def __init__(self, name: str) -> None:
        self.name: str = name.lower()
        self.votes: int = 0
    def capitalized_name(self) -> str:
        return self.name.capitalize()

class Voter:
    def __init__(self, chose: list[str]) -> None:
        self.voted: list[str] = chose[:]

def create_candidates() -> List[Candidate]:
    candidates_list = input("Candidates names (different names separated by a comma): ")
    while not candidates_list.strip() or "," not in candidates_list:
        candidates_list = input("Candidates names (separated by a comma): ")

    result: List[Candidate] = []
    seen: set[str] = set()
    for raw in candidates_list.split(","):
        name = raw.strip().lower()
        if not name or name in seen:
            continue
        seen.add(name)
        result.append(Candidate(name))
    return result

def ask_voters(num_voters: int, candidates_list: List[Candidate]) -> List[Voter]:
    voters_list: List[Voter] = []
    valid: set[str] = {c.name for c in candidates_list}
    for n in range(num_voters):
        while True:
            print("")
            tokens: list = []
            for i in range(len(valid)):
                token = (input(f"Voter {n + 1}, rank #{i + 1}: ").strip().lower())
                while token == "" or token in tokens or token not in valid:
                    if token == "i do not want to vote":
                        print("""
  _____________________
| No one cares dumbass! |
  =====================
              \\
               \\
                \\
                      _------~~-,
                   ,'            ,
                   /               \\\\
                  /                :
                 |                  '
                 |                  |
                 |                  |
                  |   _--           |
                  _| =-.     .-.   ||
                  o|/o/       _.   |
                  /  ~          \\\\ |
                (____\@)  ___~    |
                   |_===~~~.`    |
                _______.--~     |
                \\\\________       |
                         \\\\      |
                       __/-___-- -__
                      /            _ \\\\
                      """)

                    vo_str: str = ", ".join(sorted(c.capitalized_name() for c in candidates_list))
                    token = (input(f"Voter {n + 1}, rank #{i + 1} (valid options: {vo_str}): ").strip().lower())
                tokens.append(token)
            ranked: list[str] = []
            for t in tokens:
                if t in valid and t not in ranked:
                    ranked.append(t)
            if ranked:
                voters_list.append(Voter(ranked))
                break
            print(f"Invalid vote. Options: {', '.join(sorted(valid))}")
    return voters_list

def count_votes(candidates_list: list, voters_list: list) -> None:
    for candidate in candidates_list:  # Reset votes before counting
        candidate.votes = 0

    for voter in voters_list:
        for candidate in candidates_list:
            if candidate.name == voter.voted[0]:
                candidate.votes += 1
    determine_winner(candidates_list, voters_list)

def determine_winner(candidates_list: list, voters_list: list) -> None:
    if not candidates_list:
        print("No candidates.")
        return

    total = sum(c.votes for c in candidates_list)
    max_votes = max(c.votes for c in candidates_list)
    min_votes = min(c.votes for c in candidates_list)
    winners = [c for c in candidates_list if c.votes == max_votes]
    losers = [c for c in candidates_list if c.votes == min_votes]

    if total == 0:
        print("ERROR: No votes cast.")
        return

    if len(winners) == 1:
        w = winners[0]
        if (w.votes / sum(c.votes for c in candidates_list)) > 0.5:
            print_winner(winners)
            return
        get_rid_of_losers(losers, candidates_list, voters_list)
        count_votes(candidates_list, voters_list)
        return

    elif len(winners) == len(candidates_list):
        if voters_list and all(len(v.voted) == 1 for v in voters_list):
            print_winner(winners)
            return
        else:
            for v in voters_list:
                if len(v.voted) > 1:
                    v.voted.pop(0)
            count_votes(candidates_list, voters_list)

    else:
        get_rid_of_losers(losers, candidates_list, voters_list)
        count_votes(candidates_list, voters_list)

def print_winner(winners: list) -> None:
    if len(winners) == 1:
        w = winners[0]
        print(f"Winner: {w.capitalized_name()} with {w.votes} final votes")
    else:
        names = " and ".join(c.capitalized_name() for c in winners)
        print(f"Tie between: {names} with {winners[0].votes} final votes")

def get_rid_of_losers(losers: list, candidates_list: list, voters_list: list) -> None:
    for l in losers:
        if l in candidates_list:
            candidates_list.remove(l)
        for v in voters_list:
            if l.name in v.voted:
                v.voted.remove(l.name)

if __name__ == "__main__":
    candidates_l = create_candidates()

    voters: int = input("Number of voters: ")
    while not voters.isdigit():
        voters = input("Number of voters: ")
    voters = int(voters)
    voters_l = ask_voters(voters, candidates_l)

    count_votes(candidates_l, voters_l)
