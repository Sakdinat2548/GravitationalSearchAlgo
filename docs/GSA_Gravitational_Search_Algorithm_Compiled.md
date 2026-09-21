```markdown
# GSA: A Gravitational Search Algorithm

**Authors:** Esmat Rashedi, Hossein Nezamabadi-pour, Saeid Saryazdi
**Affiliation:** Department of Electrical Engineering, Shahid Bahonar University of Kerman, P.O. Box 76169-133, Kerman, Iran
**Journal:** Information Sciences 179 (2009) 2232–2248
**Article history:** Received 21 April 2008; Received in revised form 11 January 2009; Accepted 8 March 2009
**Keywords:** Optimization; Heuristic search algorithms; Gravitational Search Algorithm; Law of gravity
**DOI:** 10.1016/j.ins.2009.03.004

**Source:** GSA_A_Gravitational_Search_Algorithm.pdf

---

## Abstract

In recent years, various heuristic optimization methods have been developed. Many of these methods are inspired by swarm behaviors in nature. In this paper, a new optimization algorithm based on the law of gravity and mass interactions is introduced. In the proposed algorithm, the searcher agents are a collection of masses which interact with each other based on the Newtonian gravity and the laws of motion. The proposed method has been compared with some well-known heuristic search methods. The obtained results confirm the high performance of the proposed method in solving various nonlinear functions.

---

## 1. Introduction

**Content Type: Definition**
**Content Text:**
In solving optimization problems with a high-dimensional search space, classical optimization algorithms do not provide a suitable solution because the search space increases exponentially with the problem size; therefore solving these problems using exact techniques (such as exhaustive search) is not practical.

Over the last decades, there has been growing interest in algorithms inspired by the behaviors of natural phenomena [5,8,17,19,21,32]. These algorithms are well suited to solve complex computational problems such as optimization of objective functions [6,36], pattern recognition [24,31], control objectives [2,16,20], image processing [4,27], and filter modeling [15,23]. Various heuristic approaches have been adopted, e.g., Genetic Algorithm [32], Simulated Annealing [21], Ant Colony Search Algorithm [5], Particle Swarm Optimization [1,3,7,12,25,34], etc. These algorithms solve different optimization problems, but there is no specific algorithm that achieves the best solution for all optimization problems — some algorithms perform better on particular problems than others. Hence, searching for new heuristic optimization algorithms remains an open problem [35].

In this paper, a new optimization algorithm based on the law of gravity, namely Gravitational Search Algorithm (GSA), is proposed [28]. This algorithm is based on Newtonian gravity: "Every particle in the universe attracts every other particle with a force that is directly proportional to the product of their masses and inversely proportional to the square of the distance between them."

**Content Type: Definition**
**Content Text:**
Paper organization: Section 2 provides a brief review of heuristic algorithms. Section 3 introduces the basic aspects of gravity. Section 4 describes the Gravitational Search Algorithm (GSA) and its characteristics. Section 5 presents a comparative study, and Section 6 demonstrates the experimental results.

---

## 2. A Review of Heuristic Algorithms

**Content Type: Definition**
**Content Text:**
The word "heuristic" is Greek, meaning "to know," "to find," "to discover," or "to guide an investigation" [22]. "Heuristics are techniques which seek good (near-optimal) solutions at a reasonable computational cost without being able to guarantee either feasibility or optimality, or even in many cases to state how close to optimality a particular feasible solution is." (Russell and Norvig, 1995; [29])

Heuristic algorithms mimic physical or biological processes. Famous examples: Genetic Algorithm (GA, inspired by Darwinian evolutionary theory [32]), Simulated Annealing (SA, based on thermodynamic effects [21]), Artificial Immune System (AIS, simulating biological immune systems [8]), Ant Colony Optimization (ACO, mimicking ants foraging for food [5]), Bacterial Foraging Algorithm (BFA, from search and optimal foraging of bacteria [11,19]), and Particle Swarm Optimization (PSO, simulating the behavior of a flock of birds [3,17]).

All the above heuristic algorithms have stochastic behavior. However, Formato [9,10] proposed a deterministic heuristic search algorithm based on the metaphor of gravitational kinematics, called Central Force Optimization (CFO).

**Content Type: Definition**
**Content Text:**
In some stochastic algorithms (like SA), the search starts from a single point and proceeds sequentially; however, most heuristic algorithms search in parallel with multiple initial points — e.g., swarm-based algorithms use a collection of agents similar to a natural flock of birds or fishes. In a swarm-based algorithm, each member executes simple operations and shares information with others; the collective effect, known as swarm intelligence [5,33], produces a surprising result. Local interactions between agents produce a global result, allowing the system to solve the problem without a central controller. Member operations — randomized search, positive feedback, negative feedback, and multiple interactions — lead to a self-organization situation [5].

**Content Type: Definition**
**Content Text:**
Two common aspects recognized in population-based heuristic algorithms: exploration (the ability to expand the search space) and exploitation (the ability to find the optima around a good solution). In early iterations, a heuristic search algorithm explores the search space to find new solutions and must use exploration to avoid trapping in a local optimum; as iterations proceed, exploration fades out and exploitation fades in, so the algorithm tunes itself toward semi-optimal points. A suitable tradeoff between exploration and exploitation is key to high search performance. All population-based heuristic algorithms use exploration and exploitation but via different approaches and operators — i.e., all search algorithms share a common framework.

From another point of view, members of a population-based search algorithm pass through three steps each iteration to realize exploration and exploitation: self-adaptation (each agent improves its own performance), cooperation (members collaborate by transferring information), and competition (members compete to survive). These steps, usually stochastic and realized in different ways, are inspired from nature and are the principal ideas of population-based heuristic algorithms, guiding an algorithm toward a global optimum.

However, no heuristic algorithm provides superior performance over all others for all optimization problems — an algorithm may solve some problems better and others worse than others [35]. Hence, proposing new high-performance heuristic algorithms is welcome. This paper aims to establish a new population-based search algorithm based on gravity rules.

---

## 3. The Law of Gravity

**Content Type: Definition**
**Content Text:**
Gravitation is the tendency of masses to accelerate toward each other. It is one of the four fundamental interactions in nature [30] (the others being the electromagnetic force, the weak nuclear force, and the strong nuclear force). Every particle in the universe attracts every other particle; gravity is everywhere, and its inescapability distinguishes it from all other natural forces.

The way Newton's gravitational force behaves is called "action at a distance" — gravity acts between separated particles without any intermediary and without delay. In Newton's law of gravity, each particle attracts every other particle with a gravitational force [14,30].

**Content Type: Core Formula**
**Content Text:**
$$F = G\,\frac{M_1 M_2}{R^2} \tag{1}$$
where $F$ is the magnitude of the gravitational force, $G$ is the gravitational constant, $M_1$ and $M_2$ are the masses of the first and second particles, respectively, and $R$ is the distance between the two particles.

Newton's second law: when a force $F$ is applied to a particle, its acceleration $a$ depends only on the force and its mass $M$ [14]:
$$a = \frac{F}{M}. \tag{2}$$

**Content Type: Theorem/Property**
**Content Text:**
Based on Eqs. (1) and (2), there is an attracting gravity force among all particles of the universe, where the effect of bigger and closer particles is higher. An increase in distance between two particles decreases the gravity force between them (Fig. 1). In Fig. 1, $F_{1j}$ is the force acting on $M_1$ from $M_j$, and $F_1$ is the overall force acting on $M_1$, causing acceleration vector $a_1$.
[Diagram: Fig. 1 — four masses $M_1, M_2, M_3, M_4$; arrows $F_{12}, F_{13}, F_{14}$ show the individual pairwise forces on $M_1$ from each other mass, and the resultant force $F_1$ produces acceleration $a_1$ on $M_1$, illustrating that every mass accelerates toward the net force acting on it from the other masses.]

**Content Type: Core Formula**
**Content Text:**
Due to the effect of decreasing gravity, the actual value of the gravitational constant depends on the actual age of the universe. The decrease of the gravitational constant $G$ with age is given by [26]:
$$G(t) = G(t_0)\left(\frac{t_0}{t}\right)^{\beta}, \qquad \beta < 1, \tag{3}$$
where $G(t)$ is the value of the gravitational constant at time $t$, and $G(t_0)$ is its value at the first cosmic quantum-interval of time $t_0$ [26].

**Content Type: Definition**
**Content Text:**
Three kinds of mass are defined in theoretical physics:
- **Active gravitational mass**, $M_a$: a measure of the strength of the gravitational field due to a particular object. An object with small active gravitational mass produces a weaker gravitational field than one with more active gravitational mass.
- **Passive gravitational mass**, $M_p$: a measure of the strength of an object's interaction with the gravitational field. Within the same gravitational field, an object with smaller passive gravitational mass experiences a smaller force than one with larger passive gravitational mass.
- **Inertial mass**, $M_i$: a measure of an object's resistance to changing its state of motion when a force is applied. An object with large inertial mass changes its motion more slowly; one with small inertial mass changes it rapidly.

**Content Type: Core Formula**
**Content Text:**
Rewriting Newton's laws considering these mass types: the gravitational force $F_{ij}$ acting on mass $i$ by mass $j$ is proportional to the product of the active gravitational mass of $j$ and the passive gravitational mass of $i$, and inversely proportional to the square of the distance between them; $a_i$ is proportional to $F_{ij}$ and inversely proportional to the inertial mass of $i$:
$$F_{ij} = G\,\frac{M_{aj}\cdot M_{pi}}{R^2}, \tag{4}$$
$$a_i = \frac{F_{ij}}{M_{ii}}, \tag{5}$$
where $M_{aj}$ and $M_{pi}$ represent the active gravitational mass of particle $j$ and the passive gravitational mass of particle $i$, respectively (note: the paper's text labels these as "of particle $i$ and $j$" respectively in-line, but the subscripts as written are $M_{aj}$, $M_{pi}$), and $M_{ii}$ represents the inertia mass of particle $i$.

**Content Type: Theorem/Property**
**Content Text:**
Although inertial mass, passive gravitational mass, and active gravitational mass are conceptually distinct, no experiment has ever unambiguously demonstrated any difference between them. The theory of general relativity rests on the assumption that inertial and passive gravitational mass are equivalent — the weak equivalence principle [18]. Standard general relativity also assumes the equivalence of inertial mass and active gravitational mass — sometimes called the strong equivalence principle [18].

---

## 4. Gravitational Search Algorithm (GSA)

**Content Type: Definition**
**Content Text:**
In the proposed algorithm [28], agents are considered as objects whose performance is measured by their masses. All objects attract each other by the gravity force, which causes a global movement of all objects toward those with heavier masses. Masses cooperate using a direct form of communication through gravitational force. Heavy masses — corresponding to good solutions — move more slowly than lighter ones, guaranteeing the exploitation step of the algorithm.

In GSA, each mass (agent) has four specifications: position, inertial mass, active gravitational mass, and passive gravitational mass. The position of the mass corresponds to a solution of the problem, and its gravitational and inertial masses are determined using a fitness function. Each mass presents a solution, and the algorithm is navigated by properly adjusting the gravitational and inertia masses. Over time, masses are expected to be attracted by the heaviest mass, which will present an optimum solution in the search space.

GSA can be considered an isolated system of masses, like a small artificial world obeying Newtonian laws of gravitation and motion:
- **Law of gravity:** each particle attracts every other particle, and the gravitational force between two particles is directly proportional to the product of their masses and inversely proportional to the distance $R$ between them. (The paper uses $R$ instead of $R^2$, because — according to the authors' experimental results — $R$ gives better results than $R^2$ in all experimental cases.)
- **Law of motion:** the current velocity of any mass equals the sum of a fraction of its previous velocity and the variation in velocity. Variation in velocity (acceleration) of any mass equals the force acting on the system divided by its inertial mass.

**Content Type: Core Formula**
**Content Text:**
Consider a system with $N$ agents (masses). The position of the $i$th agent is defined by:
$$X_i = (x_i^1, \ldots, x_i^d, \ldots, x_i^n) \quad \text{for } i = 1, 2, \ldots, N, \tag{6}$$
where $x_i^d$ presents the position of the $i$th agent in the $d$th dimension.

At a specific time $t$, the force acting on mass $i$ from mass $j$ is defined as:
$$F_{ij}^d(t) = G(t)\,\frac{M_{pi}(t)\cdot M_{aj}(t)}{R_{ij}(t)+\varepsilon}\big(x_j^d(t) - x_i^d(t)\big), \tag{7}$$
where $M_{aj}$ is the active gravitational mass related to agent $j$, $M_{pi}$ is the passive gravitational mass related to agent $i$, $G(t)$ is the gravitational constant at time $t$, $\varepsilon$ is a small constant, and $R_{ij}(t)$ is the Euclidian distance between agents $i$ and $j$:
$$R_{ij}(t) = \lVert X_i(t), X_j(t) \rVert_2. \tag{8}$$

To give a stochastic characteristic to the algorithm, the total force acting on agent $i$ in dimension $d$ is a randomly weighted sum of the $d$th components of the forces exerted by other agents:
$$F_i^d(t) = \sum_{j=1,\, j\neq i}^{N} \mathrm{rand}_j\, F_{ij}^d(t), \tag{9}$$
where $\mathrm{rand}_j$ is a random number in the interval $[0,1]$.

By the law of motion, the acceleration of agent $i$ at time $t$ in direction $d$, $a_i^d(t)$, is:
$$a_i^d(t) = \frac{F_i^d(t)}{M_{ii}(t)}, \tag{10}$$
where $M_{ii}$ is the inertial mass of the $i$th agent.

The next velocity of an agent is a fraction of its current velocity plus its acceleration; position and velocity are calculated as:
$$v_i^d(t+1) = \mathrm{rand}_i \cdot v_i^d(t) + a_i^d(t), \tag{11}$$
$$x_i^d(t+1) = x_i^d(t) + v_i^d(t+1), \tag{12}$$
where $\mathrm{rand}_i$ is a uniform random variable in $[0,1]$, used to give a randomized characteristic to the search.

**Content Type: Core Formula**
**Content Text:**
The gravitational constant $G$ is initialized at the beginning and reduced over time to control search accuracy; $G$ is a function of the initial value $G_0$ and time $t$:
$$G(t) = G(G_0, t). \tag{13}$$

Gravitational and inertia masses are calculated by fitness evaluation — a heavier mass means a more efficient agent (higher attraction, slower movement). Assuming equality of gravitational and inertia mass, mass values are calculated using the map of fitness, updated by:
$$M_{ai} = M_{pi} = M_{ii} = M_i, \qquad i = 1, 2, \ldots, N, \tag{14}$$
$$m_i(t) = \frac{\mathrm{fit}_i(t) - \mathrm{worst}(t)}{\mathrm{best}(t) - \mathrm{worst}(t)}, \tag{15}$$
$$M_i(t) = \frac{m_i(t)}{\sum_{j=1}^{N} m_j(t)}, \tag{16}$$
where $\mathrm{fit}_i(t)$ represents the fitness value of agent $i$ at time $t$, and, for a minimization problem, $\mathrm{worst}(t)$ and $\mathrm{best}(t)$ are defined as:
$$\mathrm{best}(t) = \min_{j\in\{1,\ldots,N\}} \mathrm{fit}_j(t), \tag{17}$$
$$\mathrm{worst}(t) = \max_{j\in\{1,\ldots,N\}} \mathrm{fit}_j(t). \tag{18}$$
For a maximization problem, Eqs. (17) and (18) change to Eqs. (19) and (20), respectively:
$$\mathrm{best}(t) = \max_{j\in\{1,\ldots,N\}} \mathrm{fit}_j(t), \tag{19}$$
$$\mathrm{worst}(t) = \min_{j\in\{1,\ldots,N\}} \mathrm{fit}_j(t). \tag{20}$$

**Content Type: Definition**
**Content Text:**
One way to achieve a good compromise between exploration and exploitation is to reduce, over time, the number of agents whose force is applied in Eq. (9) — only a set of agents with bigger mass apply their force to the others. Care must be taken with this policy, since it may reduce exploration power and increase exploitation capability.

To avoid trapping in a local optimum, the algorithm must use exploration at the beginning; as iterations pass, exploration must fade out and exploitation must fade in. To control this, only the $K_{best}$ agents attract the others. $K_{best}$ is a function of time, with initial value $K_0$ at the beginning, decreasing with time — at the beginning all agents apply force, and as time passes $K_{best}$ decreases linearly until, at the end, just one agent applies force to the others.

**Content Type: Core Formula**
**Content Text:**
Eq. (9) is modified as:
$$F_i^d(t) = \sum_{j \in K_{best},\, j\neq i} \mathrm{rand}_j\, F_{ij}^d(t), \tag{21}$$
where $K_{best}$ is the set of the first $K$ agents with the best fitness value and biggest mass.

**Content Type: Worked Example/Proof**
**Content Text:**
The different steps of the proposed algorithm are:
(a) Search space identification.
(b) Randomized initialization.
(c) Fitness evaluation of agents.
(d) Update $G(t)$, best$(t)$, worst$(t)$, and $M_i(t)$ for $i=1,2,\ldots,N$.
(e) Calculation of the total force in different directions.
(f) Calculation of acceleration and velocity.
(g) Updating agents' position.
(h) Repeat steps c to g until the stop criteria is reached.
(i) End.

[Diagram: Fig. 2 — flowchart titled "General principle of GSA": Generate initial population → Evaluate the fitness for each agent → Update the G, best and worst of the population → Calculate M and a for each agent → Update velocity and position → decision "Meeting end of criterion?" — No loops back to fitness evaluation, Yes proceeds to → Return best solution.]

**Content Type: Theorem/Property**
**Content Text:**
Remarks noted on the efficiency of the proposed algorithm:
- Since each agent can observe the performance of the others, the gravitational force is an information-transferring tool.
- Due to the force acting on an agent from its neighborhood agents, it can "see" the space around itself.
- A heavy mass has a large effective attraction radius and hence a great intensity of attraction; agents with higher performance have greater gravitational mass, so agents tend to move toward the best agent.
- The inertia mass opposes motion and slows mass movement; agents with heavy inertia mass move slowly and search the space more locally — it can be considered an adaptive learning rate.
- The gravitational constant adjusts the accuracy of the search, decreasing with time (similar to temperature in Simulated Annealing).
- GSA is a memory-less algorithm, yet works efficiently like algorithms with memory; experimental results show a good convergence rate.
- Gravitational and inertia masses are assumed the same here, though different values could be used for some applications: a bigger inertia mass gives slower, more precise motion in the search space; a bigger gravitational mass causes higher attraction, permitting faster convergence.

---

## 5. Comparative Study

### 5.1. PSO Algorithm

**Content Type: Definition**
**Content Text:**
PSO is motivated by simulation of social behavior (flock of birds). This approach updates the population of particles by applying an operator according to fitness information obtained from the environment, so individuals of the population move toward the better solution.

**Content Type: Core Formula**
**Content Text:**
In PSO, $x_i^d$ and $v_i^d$ are calculated as follows [17]:
$$x_i^d(t+1) = x_i^d(t) + v_i^d(t+1), \tag{22}$$
$$v_i^d(t+1) = w(t)v_i^d(t) + c_1 r_{i1}\big(\mathrm{pbest}_i^d - x_i^d(t)\big) + c_2 r_{i2}\big(\mathrm{gbest}^d - x_i^d(t)\big), \tag{23}$$
where $r_{i1}$ and $r_{i2}$ are two random variables in the range $[0,1]$, $c_1$ and $c_2$ are positive constants, $w$ is the inertia weight. $X_i = (x_i^1, x_i^2, \ldots, x_i^n)$ and $V_i = (v_i^1, v_i^2, \ldots, v_i^n)$ represent position and velocity of the $i$th particle, respectively. $\mathrm{pbest}_i = (\mathrm{pbest}_i^1, \mathrm{pbest}_i^2, \ldots, \mathrm{pbest}_i^n)$ and $\mathrm{gbest} = (\mathrm{gbest}^1, \mathrm{gbest}^2, \ldots, \mathrm{gbest}^n)$ represent the best previous position of the $i$th particle and the best previous position among all particles in the population, respectively.

From Eq. (23), each particle tries to modify its position ($X_i$) using the distance between the current position and $\mathrm{pbest}_i$, and the distance between the current position and $\mathrm{gbest}$.

**Content Type: Theorem/Property**
**Content Text:**
GSA versus PSO: In both, optimization is obtained by agent movement in the search space, but the movement strategy differs:
- In PSO, an agent's direction is calculated using only two best positions, $\mathrm{pbest}_i$ and $\mathrm{gbest}$. In GSA, the agent direction is calculated based on the overall force from all other agents.
- In PSO, updating does not consider solution quality, and fitness values are not directly important in updating; in GSA, force is proportional to fitness value, so agents "see" the search space around themselves under the influence of force.
- PSO uses a kind of memory for updating velocity (via $\mathrm{pbest}_i$ and $\mathrm{gbest}$); GSA is memory-less — only the current position of agents plays a role in updating.
- In PSO, updating does not consider the distance between solutions; in GSA, force is inversely proportional to distance between solutions.
- The search ideas differ: PSO simulates the social behavior of birds; GSA is inspired by a physical phenomenon.

### 5.2. CFO Algorithm

**Content Type: Definition**
**Content Text:**
Central Force Optimization (CFO) is a deterministic multi-dimensional search algorithm modeling probes that fly through the search space under the influence of gravity [9,10]. Initial probe positions are computed deterministically.

**Content Type: Core Formula**
**Content Text:**
For initialization, the position vector array is filled with a uniform distribution of probes on each coordinate axis at time 0, according to [9]:
$$\text{for } d=1 \text{ to } n,\ \text{for } p=1 \text{ to } N_n:\quad i = p+(d-1)N_n,\quad x_i^d(0) = x_{\min}^d + (p-1)\frac{x_{\max}^d - x_{\min}^d}{N_n - 1}, \tag{24}$$
where $n$ is the dimension of the problem and $N$ is the number of probes.

In CFO, at each iteration probes are evaluated, and for each probe $M$ is calculated using the fitness function:
$$M_i(t) = \mathrm{fit}_i, \tag{25}$$
where $M_i(t)$ is the mass of probe $i$ at time $t$. Acceleration is then updated using:
$$a_i^d(t) = G\sum_{j=1,\,j\neq i}^{N} U\big(M_j(t)-M_i(t)\big)\big[M_j(t)-M_i(t)\big]^\alpha \frac{\big(x_j^d(t)-x_i^d(t)\big)}{R_{ij}(t)^\beta}, \tag{26}$$
and a new position is calculated using:
$$x_i^d(t+1) = x_i^d(t) + \tfrac12 a_i^d(t), \tag{27}$$
where $a_i^d(t)$ and $x_i^d(t)$ are the acceleration and position of probe $i$ at time $t$, $\alpha$ and $\beta$ are two constants, $G$ is the gravitational constant, $R_{ij}(t)$ is the Euclidean distance between probes $i$ and $j$ at time $t$, $U$ is the unit step function, and $\mathrm{fit}_i$ is the fitness function of probe $i$, which must be maximized.

**Content Type: Theorem/Property**
**Content Text:**
As Eqs. (25) and (26) show, in CFO mass is the difference between fitness values; since masses can be positive or negative depending on which fitness is greater, the unit step function is included to avoid a negative mass [9]. Because mass is proportionally dependent on the fitness function, in some problems acceleration may be very high and probes go out of the search space; in CFO, any probe that "flew" out of the decision space was returned to the midpoint between its past position and the minimum or maximum value of the coordinate lying outside the allowable range [9].

GSA versus CFO: in both, probe positions and accelerations are inspired by particle motion in a gravitational field, but they use different formulations:
- CFO is inherently deterministic, using no random parameter, while GSA is a stochastic search algorithm.
- The acceleration and movement expressions and mass calculations in GSA differ from CFO.
- In CFO, the initial probe distribution is systematic (a deterministic rule) with a significant effect on convergence; in GSA the initial distribution is random.
- In CFO, $G$ is a constant, while in GSA $G$ is a control parameter (decreasing over time).

---

## 6. Experimental Results

**Content Type: Definition**
**Content Text:**
To evaluate performance, the algorithm was applied to 23 standard benchmark functions [36] (Section 6.1). A comparison with Real Genetic Algorithm (RGA) and PSO is given in Section 6.2, and a comparison with CFO (deterministic) is given in Section 6.3.

### 6.1. Benchmark Functions

**Content Type: Core Formula**
**Content Text:**
**Table 1 — Unimodal test functions**

| Test function | $S$ |
|---|---|
| $F_1(X) = \sum_{i=1}^n x_i^2$ | $[-100,100]^n$ |
| $F_2(X) = \sum_{i=1}^n \lvert x_i\rvert + \prod_{i=1}^n \lvert x_i\rvert$ | $[-10,10]^n$ |
| $F_3(X) = \sum_{i=1}^n \left(\sum_{j=1}^i x_j\right)^2$ | $[-100,100]^n$ |
| $F_4(X) = \max_i\{\lvert x_i\rvert,\ 1\leqslant i\leqslant n\}$ | $[-100,100]^n$ |
| $F_5(X) = \sum_{i=1}^{n-1}\left[100\big(x_{i+1}-x_i^2\big)^2 + (x_i-1)^2\right]$ | $[-30,30]^n$ |
| $F_6(X) = \sum_{i=1}^n \big(\lfloor x_i+0.5\rfloor\big)^2$ | $[-100,100]^n$ |
| $F_7(X) = \sum_{i=1}^n i\,x_i^4 + \mathrm{random}[0,1)$ | $[-1.28,1.28]^n$ |

**Table 2 — Multimodal test functions**

| Test function | $S$ |
|---|---|
| $F_8(X) = \sum_{i=1}^n -x_i\sin\!\left(\sqrt{\lvert x_i\rvert}\right)$ | $[-500,500]^n$ |
| $F_9(X) = \sum_{i=1}^n \left[x_i^2 - 10\cos(2\pi x_i) + 10\right]$ | $[-5.12,5.12]^n$ |
| $F_{10}(X) = -20\exp\!\left(-0.2\sqrt{\tfrac1n\sum_{i=1}^n x_i^2}\right) - \exp\!\left(\tfrac1n\sum_{i=1}^n \cos(2\pi x_i)\right) + 20 + e$ | $[-32,32]^n$ |
| $F_{11}(X) = \tfrac{1}{4000}\sum_{i=1}^n x_i^2 - \prod_{i=1}^n \cos\!\left(\tfrac{x_i}{\sqrt i}\right) + 1$ | $[-600,600]^n$ |
| $F_{12}(x) = \tfrac{\pi}{n}\Big\{10\sin(\pi y_1) + \sum_{i=1}^{n-1}(y_i-1)^2\big[1+10\sin^2(\pi y_{i+1})\big] + (y_n-1)^2\Big\} + \sum_{i=1}^n u(x_i,10,100,4)$, where $y_i = 1+\dfrac{x_i+1}{4}$, and $u(x_i,a,k,m) = \begin{cases} k(x_i-a)^m & x_i>a \\ 0 & -a\leqslant x_i \leqslant a \\ k(-x_i-a)^m & x_i<-a \end{cases}$ | $[-50,50]^n$ |
| $F_{13}(X) = 0.1\Big\{\sin^2(3\pi x_1) + \sum_{i=1}^{n-1}(x_i-1)^2\big[1+\sin^2(3\pi x_{i+1})\big] + (x_n-1)^2\big[1+\sin^2(2\pi x_n)\big]\Big\} + \sum_{i=1}^n u(x_i,5,100,4)$ | $[-50,50]^n$ |

**Table 3 — Multimodal test functions with fixed dimension**

| Test function | $S$ |
|---|---|
| $F_{14}(X) = \left[\dfrac{1}{500} + \sum_{j=1}^{25}\dfrac{1}{j+\sum_{i=1}^2(x_i-a_{ij})^6}\right]^{-1}$ | $[-65.53,65.53]^2$ |
| $F_{15}(X) = \sum_{i=1}^{11}\left[a_i - \dfrac{x_1(b_i^2+b_ix_2)}{b_i^2+b_ix_3+x_4}\right]^2$ | $[-5,5]^4$ |
| $F_{16}(X) = 4x_1^2 - 2.1x_1^4 + \tfrac13 x_1^6 + x_1x_2 - 4x_2^2 + 4x_2^4$ | $[-5,5]^2$ |
| $F_{17}(X) = \left(x_2 - \dfrac{5.1}{4\pi^2}x_1^2 + \dfrac{5}{\pi}x_1 - 6\right)^2 + 10\left(1-\dfrac{1}{8\pi}\right)\cos x_1 + 10$ | $[-5,10]\times[0,15]$ |
| $F_{18}(X) = \big[1+(x_1+x_2+1)^2(19-14x_1+3x_1^2-14x_2+6x_1x_2+3x_2^2)\big]\times\big[30+(2x_1-3x_2)^2(18-32x_1+12x_1^2+48x_2-36x_1x_2+27x_2^2)\big]$ | $[-5,5]^2$ |
| $F_{19}(X) = \sum_{i=1}^4 c_i\exp\!\left(-\sum_{j=1}^3 a_{ij}(x_j-p_{ij})^2\right)$ | $[0,1]^3$ |
| $F_{20}(X) = \sum_{i=1}^4 c_i\exp\!\left(-\sum_{j=1}^6 a_{ij}(x_j-p_{ij})^2\right)$ | $[0,1]^6$ |
| $F_{21}(X) = \sum_{i=1}^5 \left[(X-a_i)(X-a_i)^T + c_i\right]^{-1}$ | $[0,10]^4$ |
| $F_{22}(X) = \sum_{i=1}^7 \left[(X-a_i)(X-a_i)^T + c_i\right]^{-1}$ | $[0,10]^4$ |
| $F_{23}(X) = \sum_{i=1}^{10} \left[(X-a_i)(X-a_i)^T + c_i\right]^{-1}$ | $[0,10]^4$ |

**Content Type: Definition**
**Content Text:**
$n$ is the dimension of the function, $f_{opt}$ is the minimum value of the function, and $S$ is a subset of $\mathbb{R}^n$. The minimum value ($f_{opt}$) of the functions of Tables 1 and 2 is zero, except for $F_8$, which has minimum value $-418.9829\,n$. The optimum location ($X_{opt}$) for functions of Tables 1 and 2 is $[0]^n$, except for $F_5, F_{12}, F_{13}$ with $X_{opt}$ in $[1]^n$, and $F_8$ in $[-420.96]^n$. A detailed description of the functions of Table 3 is given in Appendix A.

### 6.2. A Comparison with PSO and RGA

**Content Type: Worked Example/Proof**
**Content Text:**
GSA was applied to these minimization functions and compared with RGA and PSO results. In all cases, population size was set to 50 ($N=50$), dimension 30 ($n=30$), and maximum iterations 1000 for functions of Tables 1 and 2, and 500 for functions of Table 3. In PSO, $c_1=c_2=2$, and the inertia factor $w$ decreases linearly from 0.9 to 0.2. In RGA, arithmetic crossover, Gaussian mutation, and roulette-wheel selection were used as described in [13], with crossover and mutation probabilities set to 0.3 and 0.1, respectively.

In GSA, $G$ is set using:
$$G(t) = G_0 e^{-\alpha t/T}, \tag{28}$$
where $G_0$ is set to 100, $\alpha$ is set to 20, and $T$ is the total number of iterations (the total age of the system). $K_0$ is set to $N$ (total number of agents) and decreased linearly to 1.

**Content Type: Worked Example/Proof**
**Content Text:**
**Unimodal high-dimensional functions.** Functions $F_1$ to $F_7$ are unimodal. Here the convergence rate is more important than the final results, since other methods are specifically designed to optimize unimodal functions. Results are averaged over 30 runs.

**Table 4 — Minimization result of benchmark functions in Table 1 with $n=30$. Maximum number of iterations = 1000.**

| | | RGA | PSO | GSA |
|---|---|---|---|---|
| $F_1$ | Average best-so-far | 23.13 | $1.8\times10^{-3}$ | $7.3\times10^{-11}$ |
| | Median best-so-far | 21.87 | $1.2\times10^{-3}$ | $7.1\times10^{-11}$ |
| | Average mean fitness | 23.45 | $5.0\times10^{-2}$ | $2.1\times10^{-10}$ |
| $F_2$ | Average best-so-far | 1.07 | 2.0 | $4.03\times10^{-5}$ |
| | Median best-so-far | 1.13 | $1.9\times10^{-3}$ | $4.07\times10^{-5}$ |
| | Average mean fitness | 1.07 | 2.0 | $6.9\times10^{-5}$ |
| $F_3$ | Average best-so-far | $5.6\times10^{+3}$ | $4.1\times10^{+3}$ | $0.16\times10^{+3}$ |
| | Median best-so-far | $5.6\times10^{+3}$ | $2.2\times10^{+3}$ | $0.15\times10^{+3}$ |
| | Average mean fitness | $5.6\times10^{+3}$ | $2.9\times10^{+3}$ | $0.16\times10^{+3}$ |
| $F_4$ | Average best-so-far | 11.78 | 8.1 | $3.7\times10^{-6}$ |
| | Median best-so-far | 11.94 | 7.4 | $3.7\times10^{-6}$ |
| | Average mean fitness | 11.78 | 23.6 | $8.5\times10^{-6}$ |
| $F_5$ | Average best-so-far | $1.1\times10^{+3}$ | $3.6\times10^{+4}$ | 25.16 |
| | Median best-so-far | $1.0\times10^{+3}$ | $1.7\times10^{+3}$ | 25.18 |
| | Average mean fitness | $1.1\times10^{+3}$ | $3.7\times10^{+4}$ | 25.16 |
| $F_6$ | Average best-so-far | 24.01 | $1.0\times10^{-3}$ | $8.3\times10^{-11}$ |
| | Median best-so-far | 24.55 | $6.6\times10^{-3}$ | $7.7\times10^{-11}$ |
| | Average mean fitness | 24.52 | 0.02 | $2.6\times10^{-10}$ |
| $F_7$ | Average best-so-far | 0.06 | 0.04 | 0.018 |
| | Median best-so-far | 0.06 | 0.04 | 0.015 |
| | Average mean fitness | 0.56 | 1.04 | 0.533 |

[Diagram: Fig. 3 — semi-log line chart, y-axis "Average best-so-far" from $10^{-15}$ to $10^5$, x-axis "Iteration" from 0 to 1000, three curves (GSA solid, PSO dashed, RGA dash-dot) for minimization of $F_1$ with $n=30$; GSA descends fastest and furthest, ending near $10^{-11}$–$10^{-15}$, while PSO and RGA plateau much higher.]

**Content Type: Worked Example/Proof**
**Content Text:**
As Table 4 illustrates, GSA provides better results than RGA and PSO for all unimodal functions, with the largest performance difference occurring on these unimodal functions due to GSA's attractive power. The good convergence rate of GSA is also seen in Figs. 3 and 4: GSA tends to find the global optimum faster than the other algorithms and hence has a higher convergence rate.

[Diagram: Fig. 4 — semi-log line chart, y-axis "Average best-so-far" ($10^{-2}$ to $10^3$), x-axis "Iteration" (0–1000), comparing GSA, PSO, RGA for minimization of $F_7$ with $n=30$; GSA plateaus at a high value initially then drops sharply around iteration 300–400 to below RGA and PSO, ending near $10^{-2}$.]

**Content Type: Worked Example/Proof**
**Content Text:**
**Multimodal high-dimensional functions.** Multimodal functions have many local minima and are among the most difficult to optimize; for these, the final results matter more than convergence rate, since they reflect the algorithm's ability to escape poor local optima and locate a near-global optimum. Experiments were carried out on $F_8$ to $F_{13}$, where the number of local minima increases exponentially with dimension (set to 30 here). Results averaged over 30 runs are in Table 5.

**Table 5 — Minimization result of benchmark functions in Table 2 with $n=30$. Maximum number of iterations = 1000.**

| | | RGA | PSO | GSA |
|---|---|---|---|---|
| $F_8$ | Average best-so-far | $-1.2\times10^{+4}$ | $-9.8\times10^{+3}$ | $-2.8\times10^{+3}$ |
| | Median best-so-far | $-1.2\times10^{+4}$ | $-9.8\times10^{+3}$ | $-2.6\times10^{+3}$ |
| | Average mean fitness | $-1.2\times10^{+4}$ | $-9.8\times10^{+3}$ | $-1.1\times10^{+3}$ |
| $F_9$ | Average best-so-far | 5.90 | 55.1 | 15.32 |
| | Median best-so-far | 5.71 | 56.6 | 14.42 |
| | Average mean fitness | 5.92 | 72.8 | 15.32 |
| $F_{10}$ | Average best-so-far | 2.13 | $9.0\times10^{-3}$ | $6.9\times10^{-6}$ |
| | Median best-so-far | 2.16 | $6.0\times10^{-3}$ | $6.9\times10^{-6}$ |
| | Average mean fitness | 2.15 | 0.02 | $1.1\times10^{-5}$ |
| $F_{11}$ | Average best-so-far | 1.16 | 0.01 | 0.29 |
| | Median best-so-far | 1.14 | 0.0081 | 0.04 |
| | Average mean fitness | 1.16 | 0.055 | 0.29 |
| $F_{12}$ | Average best-so-far | 0.051 | 0.29 | 0.01 |
| | Median best-so-far | 0.039 | 0.11 | $4.2\times10^{-13}$ |
| | Average mean fitness | 0.053 | $9.3\times10^{+3}$ | 0.01 |
| $F_{13}$ | Average best-so-far | 0.081 | $3.1\times10^{-18}$ | $3.2\times10^{-32}$ |
| | Median best-so-far | 0.032 | $2.2\times10^{-23}$ | $2.3\times10^{-32}$ |
| | Average mean fitness | 0.081 | $4.8\times10^{+5}$ | $3.2\times10^{-32}$ |

**Content Type: Worked Example/Proof**
**Content Text:**
For $F_{10}, F_{12}$, and $F_{13}$, GSA performs much better than the others; however, for $F_8$, GSA cannot tune itself and does not perform well. The progress of the average best-so-far solution over 30 runs for $F_{10}$ and $F_{12}$ is shown in Figs. 5 and 6.

[Diagram: Fig. 5 — semi-log line chart, y-axis "Average best-so-far" ($10^{-6}$ to $10^2$), x-axis "Iteration" (0–1000), GSA/PSO/RGA for minimization of $F_{10}$, $n=30$; GSA and another curve start near 10–20 then GSA descends steadily to about $10^{-5}$–$10^{-6}$ by iteration 1000, well below the others which plateau around 1–2.]

[Diagram: Fig. 6 — semi-log line chart, y-axis "Average best-so-far" ($10^{-2}$ to $10^{10}$), x-axis "Iteration" (0–1000), GSA/PSO/RGA for minimization of $F_{12}$, $n=30$; GSA drops sharply within the first ~250 iterations to about $10^{-2}$, while PSO and RGA descend more gradually and plateau much higher (RGA around 1, PSO fluctuating with a late drop around iteration 500–550).]

**Content Type: Worked Example/Proof**
**Content Text:**
**Multimodal low-dimensional functions.** Table 6 compares GSA, RGA, and PSO on the multimodal low-dimensional benchmark functions of Table 3. Results show that RGA, PSO, and GSA have similar solutions, with performances almost the same, as confirmed by Figs. 7 and 8.

**Table 6 — Minimization result of benchmark functions in Table 3. Maximum number of iterations = 500.**

| | | RGA | PSO | GSA |
|---|---|---|---|---|
| $F_{14}$, $n=2$ | Average best-so-far | 0.998 | 0.998 | 3.70 |
| | Median best-so-far | 0.998 | 0.998 | 2.07 |
| | Average mean fitness | 0.998 | 0.998 | 9.177 |
| $F_{15}$, $n=4$ | Average best-so-far | $4.0\times10^{-3}$ | $2.8\times10^{-3}$ | $8.0\times10^{-3}$ |
| | Median best-so-far | $1.7\times10^{-3}$ | $7.1\times10^{-4}$ | $7.4\times10^{-4}$ |
| | Average mean fitness | $4.0\times10^{-3}$ | 215.60 | $9.0\times10^{-3}$ |
| $F_{16}$, $n=2$ | Average best-so-far | $-1.0313$ | $-1.0316$ | $-1.0316$ |
| | Median best-so-far | $-1.0315$ | $-1.0316$ | $-1.0316$ |
| | Average mean fitness | $-1.0313$ | $-1.0316$ | $-1.0316$ |
| $F_{17}$, $n=2$ | Average best-so-far | 0.3996 | 0.3979 | 0.3979 |
| | Median best-so-far | 0.3980 | 0.3979 | 0.3979 |
| | Average mean fitness | 0.3996 | 2.4112 | 0.3979 |
| $F_{18}$, $n=2$ | Average best-so-far | 5.70 | 3.0 | 3.0 |
| | Median best-so-far | 3.0 | 3.0 | 3.0 |
| | Average mean fitness | 5.70 | 3.0 | 3.0 |
| $F_{19}$, $n=3$ | Average best-so-far | $-3.8627$ | $-3.8628$ | $-3.7357$ |
| | Median best-so-far | $-3.8628$ | $-3.8628$ | $-3.8628$ |
| | Average mean fitness | $-3.8627$ | $-3.8628$ | $-3.8020$ |
| $F_{20}$, $n=6$ | Average best-so-far | $-3.3099$ | $-3.2369$ | $-2.0569$ |
| | Median best-so-far | $-3.3217$ | $-3.2031$ | $-1.9946$ |
| | Average mean fitness | $-3.3098$ | $-3.2369$ | $-1.6014$ |
| $F_{21}$, $n=4$ | Average best-so-far | $-5.6605$ | $-6.6290$ | $-6.0748$ |
| | Median best-so-far | $-2.6824$ | $-5.1008$ | $-5.0552$ |
| | Average mean fitness | $-5.6605$ | $-5.7496$ | $-6.0748$ |
| $F_{22}$, $n=4$ | Average best-so-far | $-7.3421$ | $-9.1118$ | $-9.3399$ |
| | Median best-so-far | $-10.3932$ | $-10.402$ | $-10.402$ |
| | Average mean fitness | $-7.3421$ | $-9.9305$ | $-9.3399$ |
| $F_{23}$, $n=4$ | Average best-so-far | $-6.2541$ | $-9.7634$ | $-9.4548$ |
| | Median best-so-far | $-4.5054$ | $-10.536$ | $-10.536$ |
| | Average mean fitness | $-6.2541$ | $-8.7626$ | $-9.4548$ |

[Diagram: Fig. 7 — line chart, y-axis "Average best-so-far" (0 to 0.16), x-axis "Iteration" (0–500), GSA/PSO/RGA for minimization of $F_{15}$, $n=4$; all three curves drop rapidly within the first ~50 iterations from about 0.14 to near 0.005–0.01 and then stay roughly flat.]

[Diagram: Fig. 8 — line chart, y-axis "Average best-so-far" (−10 to 0), x-axis "Iteration" (0–500), GSA/PSO/RGA for minimization of $F_{22}$, $n=4$; GSA descends fastest to about −9.3 by iteration ~150 and stays there, PSO reaches a similar plateau slightly later, RGA plateaus higher near −7.3.]

### 6.3. Comparison with CFO

**Content Type: Theorem/Property**
**Content Text:**
Based on the theoretical comparison, it is very difficult to compare GSA and CFO under the same conditions: CFO's performance is highly dependent on initial population generation and population size, becoming more sensitive to these as problem complexity/dimension increases. To obtain acceptable CFO results, it must start with a large population; this means before solving a problem with CFO one must know some a priori information about its complexity to set the population size or run the algorithm many times to find a suitable value by trial and error. GSA, in contrast, is a stochastic search algorithm that can optimize a wide range of problems with a fixed, small population size.

For this reason, GSA and CFO are compared here in low-dimensional problems rather than high-dimensional ones under identical conditions. Since CFO searches for maxima, the negative of the functions is tested for CFO; per [9], to avoid bias from the location of maxima relative to the initial probe distribution, maxima were offset if necessary. In CFO the position vector array is filled with a uniform distribution of probes on each coordinate axis at step 0; in GSA, initialization is random. The number of agents and maximum iterations are set to 60 and 500, respectively (60 agents because CFO needs a large population size). GSA parameters are as in the previous section; for CFO, $G=2$, $\alpha=2$, $\beta=2$ [9]. GSA and CFO are compared only for the functions reported by Formato [9], since a priori information about the function is needed to apply CFO.

**Content Type: Worked Example/Proof**
**Content Text:**
Results of GSA are averaged over 30 runs; CFO is deterministic and hence has the same results for each run. Results are given in Table 7, which also reports CFO with random initialization (not uniform-distribution initialization), averaged over 30 runs.

**Table 7 — Minimization result of some functions in Tables 1–3. Maximum number of iterations = 500. Results for GSA and CFO with random initialization are averaged over 30 runs.**

| | $F_1$ | $F_5$ | $F_6$ | $F_8$ | $F_9$ | $F_{10}$ | $F_{11}$ | $F_{14}$ | $F_{16}$ | $F_{17}$ |
|---|---|---|---|---|---|---|---|---|---|---|
| $n$ (dimension) | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 2 | 2 | 2 |
| GSA | $\mathbf{3.1\times10^{-19}}$ | 1.62 | $\mathbf{3.3\times10^{-19}}$ | $-1.08\times10^{+3}$ | $\mathbf{0.99}$ | $\mathbf{1.1\times10^{-9}}$ | 0.61 | $\mathbf{2.87}$ | $-1.031$ | $\mathbf{0.39}$ |
| CFO | 1.13 | $\mathbf{0.02}$ | 0.093 | $-1.37\times10^{+3}$ | 1.09 | 1.13 | $\mathbf{0.02}$ | 3.18 | $-1.025$ | 0.41 |
| CFO with random initialization | 880.18 | $1.22\times10^{+5}$ | $1.03\times10^{+3}$ | $-1.94\times10^{+3}$ | 23.52 | 2.81 | 9.46 | 8.88 | $-0.98$ | 0.46 |

*(Bold values as emphasized in the original table indicate the best result per column among GSA/CFO/CFO-random.)*

**Content Type: Worked Example/Proof**
**Content Text:**
As Table 7 shows, CFO does not give good results with random initialization, and the initialization criterion has a significant effect on the results obtained. Performance of each algorithm is shown in Figs. 9–11 for three functions. Results show GSA has a better solution than CFO except for functions $F_5$, $F_8$, and $F_{11}$.

[Diagram: Fig. 9 — semi-log line chart, y-axis "Average best-so-far" ($10^{-20}$ to $10^5$), x-axis "Iteration" (0–500), GSA/CFO/CFO-random-init for minimization of $F_1$, $n=5$; GSA descends continuously from about $10^5$ to below $10^{-15}$, while CFO and CFO-random-init remain essentially flat at around $1$ and $3\times10^3$ respectively.]

[Diagram: Fig. 10 — semi-log line chart, y-axis "Average best-so-far" ($10^{-2}$ to $10^{10}$), x-axis "Iteration" (0–500), GSA/CFO/CFO-random-init for minimization of $F_5$, $n=5$; GSA descends gradually from about $10^8$ to roughly 1–2 by iteration 500; CFO plateaus near 1 with a sharp drop around iteration 330–350 down toward $10^{-2}$; CFO-random-init stays flat around $10^5$.]

[Diagram: Fig. 11 — line chart, y-axis "Average best-so-far" (−1.2 to 0.2), x-axis "Iteration" (0–500), GSA/CFO/CFO-random-init for minimization of $F_{16}$, $n=2$; all three converge quickly (within ~50 iterations) to near −1, with CFO-random-init settling slightly higher (~−0.95) than GSA and CFO (~−1.03).]

**Content Type: Worked Example/Proof**
**Content Text:**
For optimization of high-dimensional functions, CFO should be run with large numbers of probes. Due to CFO's deterministic characteristics, it converges within the first few iterations. Simulations of some functions with 30 dimensions were reported in [9]; the reported results for dimension 30, with different numbers of agents ($N$), are summarized in Table 8.

**Table 8 — Optimization result of some functions of Tables 1–3 with CFO reported in [9]. Negative of results are reported.**

| | $F_1$ | $F_5$ | $F_6$ | $F_8$ | $F_9$ | $F_{10}$ | $F_{11}$ | $F_{14}$ | $F_{16}$ | $F_{17}$ |
|---|---|---|---|---|---|---|---|---|---|---|
| $n$ | 30 | 30 | 30 | 30 | 30 | 30 | 30 | 2 | 2 | 2 |
| $N$ [9] | 15000 | 60 | 600 | 240 | 600 | 780 | 780 | 240 | 260 | 400 |
| Iterations [9] | 2 | 250 | 4 | 8 | 8 | 5 | 6 | 2 | 16 | 18 |
| Best so far [9] | 0.0836 | 3.8 | 1 | $-12569.1$ | 30.53 | 1 | 0.045 | 1.2 | $-1.027$ | 0.398 |

**Content Type: Worked Example/Proof**
**Content Text:**
Comparing these Table 8 results with the average best-so-far of GSA in Tables 4–6, GSA provides better solutions except for $F_5$, $F_{11}$, and $F_{14}$. CFO converges in the first few iterations and cannot tune itself in the local optimum.

---

## 7. Conclusion

**Content Type: Theorem/Property**
**Content Text:**
In this article, a new optimization algorithm called Gravitational Search Algorithm (GSA) is introduced, constructed based on the law of gravity and the notion of mass interactions, using the theory of Newtonian physics with searcher agents as a collection of masses. In GSA, an isolated system of masses is considered; using the gravitational force, every mass in the system can "see" the situation of other masses, so gravitational force is a way of transferring information between different masses.

To evaluate the algorithm, it was examined on a set of various standard benchmark functions. Results obtained by GSA in most cases provide superior results, and in all cases are comparable with PSO, RGA, and CFO. Finally, it is noted that a local search — as used in [12] — after finding a result may be beneficial.

---

## Acknowledgements

The authors thank the INS Editorial Board and the anonymous reviewers for their helpful suggestions. The authors also give thanks to Dr. Richard Formato for proposing useful references and providing his CFO programs, and thank Dr. Saeid Seydnejad for proofreading the manuscript and providing valuable comments.

---

## Appendix A

**Content Type: Core Formula**
**Content Text:**
**Table A.1 — $a_{ij}$ in $F_{14}$**

$$(a_{ij}) = \begin{pmatrix} -32 & -16 & 0 & 16 & 32 & -32 & \cdots & 0 & 16 & 32 \\ -32 & -32 & -32 & -32 & -32 & -16 & \cdots & 32 & 32 & 32 \end{pmatrix}$$

**Table A.2 — $a_i$ and $b_i$ in $F_{15}$**

| $i$ | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| $a_i$ | 0.1957 | 0.1947 | 0.1735 | 0.1600 | 0.0844 | 0.0627 | 0.0456 | 0.0342 | 0.0342 | 0.0235 | 0.0246 |
| $b_i^{-1}$ | 0.25 | 0.5 | 1 | 2 | 4 | 6 | 8 | 10 | 12 | 14 | 16 |

**Table A.3 — $a_{ij}$ and $c_i$ in $F_{19}$**

| $i$ | $a_{ij}$, $j=1,2,3$ | $c_i$ |
|---|---|---|
| 1 | 3, 10, 30 | 1 |
| 2 | 0.1, 10, 35 | 1.2 |
| 3 | 3, 10, 30 | 3 |
| 4 | 0.1, 10, 30 | 3.2 |

**Table A.4 — $P_{ij}$ in $F_{19}$**

| $i$ | $P_{ij}$, $j=1,2,3$ |
|---|---|
| 1 | 0.3689, 0.1170, 0.2673 |
| 2 | 0.4699, 0.4387, 0.7470 |
| 3 | 0.1091, 0.8732, 0.5547 |
| 4 | 0.03815, 0.5743, 0.8828 |

**Table A.5 — $a_{ij}$ and $c_i$ in $F_{20}$**

| $i$ | $a_{ij}$, $j=1,\ldots,6$ | $c_i$ |
|---|---|---|
| 1 | 10, 3, 17, 3.5, 1.7, 8 | 1 |
| 2 | 0.05, 10, 17, 0.1, 8, 14 | 1.2 |
| 3 | 3, 3.5, 1.7, 10, 17, 8 | 3 |
| 4 | 17, 8, 0.05, 10, 0.1, 14 | 3.2 |

**Table A.6 — $P_{ij}$ in $F_{20}$**

| $i$ | $P_{ij}$, $j=1,\ldots,6$ |
|---|---|
| 1 | 0.131, 0.169, 0.556, 0.012, 0.828, 0.588 |
| 2 | 0.232, 0.413, 0.830, 0.373, 0.100, 0.999 |
| 3 | 0.234, 0.141, 0.352, 0.288, 0.304, 0.665 |
| 4 | 0.404, 0.882, 0.873, 0.574, 0.109, 0.038 |

**Table A.7 — $a_{ij}$ and $c_i$ in $F_{21}$, $F_{22}$, and $F_{23}$**

| $i$ | $a_{ij}$, $j=1,2,3,4$ | $c_i$ |
|---|---|---|
| 1 | 4, 4, 4, 4 | 0.1 |
| 2 | 1, 1, 1, 1 | 0.2 |
| 3 | 8, 8, 8, 8 | 0.2 |
| 4 | 6, 6, 6, 6 | 0.4 |
| 5 | 3, 7, 3, 7 | 0.4 |
| 6 | 2, 9, 2, 9 | 0.6 |
| 7 | 5, 5, 3, 3 | 0.3 |
| 8 | 8, 1, 8, 1 | 0.7 |
| 9 | 6, 2, 6, 2 | 0.5 |
| 10 | 7, 3.6, 7, 3.6 | 0.5 |

*(Note: the source PDF's raw text extraction merges each row's four entries into two repeated pairs, e.g. "44 44", "11 11", "88 88", "66 66", "37 37", "29 29", "55 33", "81 81", "62 62", "7 3.6 7 3.6" — reconstructed above as four separate values per row consistent with the standard Shekel-function coefficient table.)*

**Table A.8 — Optima in functions of Table 3**

| $F$ | $X_{opt}$ | $f_{opt}$ |
|---|---|---|
| $F_{14}$ | $(-32,-32)$ | 1 |
| $F_{15}$ | $(0.1928, 0.1908, 0.1231, 0.1358)$ | 0.00030 |
| $F_{16}$ | $(0.089,-0.712), (-0.089, 0.712)$ | $-1.0316$ |
| $F_{17}$ | $(-3.14, 12.27), (3.14, 2.275), (9.42, 2.42)$ | 0.398 |
| $F_{18}$ | $(0,-1)$ | 3 |
| $F_{19}$ | $(0.114, 0.556, 0.852)$ | $-3.86$ |
| $F_{20}$ | $(0.201, 0.15, 0.477, 0.275, 0.311, 0.657)$ | $-3.32$ |
| $F_{21}$ | 5 local minima in $a_{ij}$, $i=1,\ldots,5$ | $-10.1532$ |
| $F_{22}$ | 7 local minima in $a_{ij}$, $i=1,\ldots,7$ | $-10.4028$ |
| $F_{23}$ | 10 local minima in $a_{ij}$, $i=1,\ldots,10$ | $-10.5363$ |

---

## References

[1] A. Badr, A. Fahmy, A proof of convergence for ant algorithms, Information Sciences 160 (2004) 267–279.
[2] Z. Baojiang, L. Shiyong, Ant colony optimization algorithm and its application to neuro-fuzzy controller design, Journal of Systems Engineering and Electronics 18 (2007) 603–610.
[3] F.V.D. Bergh, A.P. Engelbrecht, A study of particle swarm optimization particle trajectories, Information Sciences 176 (2006) 937–971.
[4] O. Cordon, S. Damas, J. Santamarı´a, A fast and accurate approach for 3D image registration using the scatter search evolutionary algorithm, Pattern Recognition Letters 27 (2006) 1191–1200.
[5] M. Dorigo, V. Maniezzo, A. Colorni, The ant system: optimization by a colony of cooperating agents, IEEE Transactions on Systems, Man, and Cybernetics – Part B 26(1) (1996) 29–41.
[6] W. Du, B. Li, Multi-strategy ensemble particle swarm optimization for dynamic optimization, Information Sciences 178 (2008) 3096–3109.
[7] I. Ellabib, P. Calamai, O. Basir, Exchange strategies for multiple ant colony system, Information Sciences 177 (2007) 1248–1264.
[8] J.D. Farmer, N.H. Packard, A.S. Perelson, The immune system, adaptation and machine learning, Physica D 2 (1986) 187–204.
[9] R.A. Formato, Central force optimization: a new metaheuristic with applications in applied electromagnetics, Progress in Electromagnetics Research 77 (2007) 425–491.
[10] R.A. Formato, Central force optimization: a new nature inspired computational framework for multidimensional search and optimization, Studies in Computational Intelligence 129 (2008) 221–238.
[11] V. Gazi, K.M. Passino, Stability analysis of social foraging swarms, IEEE Transactions on Systems, Man, and Cybernetics – Part B 34(1) (2004) 539–557.
[12] C. Hamzaçebi, Improving genetic algorithms' performance by local search for continuous function optimization, Applied Mathematics and Computation 196(1) (2008) 309–317.
[13] R.L. Haupt, E. Haupt, Practical Genetic Algorithms, second ed., John Wiley & Sons, 2004.
[14] D. Holliday, R. Resnick, J. Walker, Fundamentals of physics, John Wiley and Sons, 1993.
[15] A. Kalinlia, N. Karabogab, Artificial immune algorithm for IIR filter design, Engineering Applications of Artificial Intelligence 18 (2005) 919–929.
[16] C. Karakuzu, Fuzzy controller training using particle swarm optimization for nonlinear system control, ISA Transactions 47(2) (2008) 229–239.
[17] J. Kennedy, R.C. Eberhart, Particle swarm optimization, in: Proceedings of IEEE International Conference on Neural Networks, vol. 4, 1995, pp. 1942–1948.
[18] I.R. Kenyon, General Relativity, Oxford University Press, 1990.
[19] D.H. Kim, A. Abraham, J.H. Cho, A hybrid genetic algorithm and bacterial foraging approach for global optimization, Information Sciences 177 (2007) 3918–3937.
[20] T.H. Kim, I. Maruta, T. Sugie, Robust PID controller tuning based on the constrained particle swarm optimization, Automatica 44 (2008) 1104–1110.
[21] S. Kirkpatrick, C.D. Gelatto, M.P. Vecchi, Optimization by simulated annealing, Science 220 (1983) 671–680.
[22] A. Lazar, R.G. Reynolds, Heuristic knowledge discovery for archaeological data using genetic algorithms and rough sets, Artificial Intelligence Laboratory, Department of Computer Science, Wayne State University, 2003.
[23] Y.L. Lin, W.D. Chang, J.G. Hsieh, A particle swarm optimization approach to nonlinear rational filter modeling, Expert Systems with Applications 34 (2008) 1194–1199.
[24] Y. Liu, Z. Yi, H. Wu, M. Ye, K. Chen, A tabu search approach for the minimum sum-of-squares clustering problem, Information Sciences 178 (2008) 2680–2704.
[25] M. Lozano, F. Herrera, J.R. Cano, Replacement strategies to preserve useful diversity in steady-state genetic algorithms, Information Sciences 178 (2008) 4421–4433.
[26] R. Mansouri, F. Nasseri, M. Khorrami, Effective time variation of G in a model universe with variable space dimension, Physics Letters 259 (1999) 194–200.
[27] H. Nezamabadi-pour, S. Saryazdi, E. Rashedi, Edge detection using ant algorithms, Soft Computing 10 (2006) 623–628.
[28] E. Rashedi, Gravitational Search Algorithm, M.Sc. Thesis, Shahid Bahonar University of Kerman, Kerman, Iran, 2007 (in Farsi).
[29] S.J. Russell, P. Norvig, Artificial Intelligence a Modern Approach, Prentice Hall, Upper Saddle River, New Jersey, 1995.
[30] B. Schutz, Gravity from the Ground Up, Cambridge University Press, 2003.
[31] X. Tan, B. Bhanu, Fingerprint matching by genetic algorithms, Pattern Recognition 39 (2006) 465–477.
[32] K.S. Tang, K.F. Man, S. Kwong, Q. He, Genetic algorithms and their applications, IEEE Signal Processing Magazine 13(6) (1996) 22–37.
[33] P. Tarasewich, P.R. McMullen, Swarm intelligence: power in numbers, Communication of ACM 45 (2002) 62–67.
[34] P.K. Tripathi, S. Bandyopadhyay, S.K. Pal, Multi-objective particle swarm optimization with time variant inertia and acceleration coefficients, Information Sciences 177 (2007) 5033–5049.
[35] D.H. Wolpert, W.G. Macready, No free lunch theorems for optimization, IEEE Transactions on Evolutionary Computation 1 (1997) 67–82.
[36] X. Yao, Y. Liu, G. Lin, Evolutionary programming made faster, IEEE Transactions on Evolutionary Computation 3 (1999) 82–102.
```
