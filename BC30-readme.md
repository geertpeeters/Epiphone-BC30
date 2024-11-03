<div id="header">

</div>

<div id="content">

<div class="sect1">

## General feature set

<div class="sectionbody">

<div class="ulist">

- Ampplifier with an "all tube" signal path

- Tube rectifier

- Two distinct channels

- 3 way equalizer

- Overdrive channel mid control

- Independent and interactive EQ mode

- Tube powered and transformer driven reverb

- 30W class AB (pentode) mode

- 15W Class A (triode) mode

- Multiple speaker output jacks (2x4Ohm, 2x8Ohm, 1x16Ohm)

- "Lady Luck" speakers specially designed for this amplifier

</div>

</div>

</div>

<div class="sect1">

## Overall desired improvements

<div class="sectionbody">

<div class="ulist">

- overall "harsh" clean and overdriven sounds

- perceived as "too bright" with lack of bottom end

- not switchable between clean and overdriven channel without tweaking
  the EQ settings

</div>

</div>

</div>

<div class="sect1">

## Dismantling instructions

<div class="sectionbody">

<div class="paragraph">

To remove the chassis do the following

</div>

<div class="olist arabic">

1.  Remove the mains power cable

2.  Remove the guitar cable from the input

3.  Remove the speaker cable from the output

4.  Remove the reverb RCA cables from the underside of the chassis

5.  Pry off the 4 plastic covers on top of the amp with a small flat
    blade screwdriver

6.  Remove the machine screws with a \#2 phillips screwdriver

7.  Grab the HT and OT and gently pull out while lifting up

8.  Connect one lead of an alligator clip to the chassis, and the other
    end to the common of a multimeter. probe the power tube solder
    joints making sure to leave one hand in your pocket and confirm the
    meter reads 0V

</div>

</div>

</div>

<div class="sect1">

## Affected components

<div class="sectionbody">

<div class="sect2">

### Capacitors

| Component | Identification | Actual Value | Modification                              | Expected result |
|-----------|----------------|--------------|-------------------------------------------|-----------------|
| **C3**    | 471/1kV        | 470pF/1000V  | Unsolder or cut off                       |                 |
| **C4**    | 101/1kV        | 100pF/1000V  | Unsolder or cut off                       |                 |
| **C5**    | 471/400V       | 470pF/400V   | Unsolder or cut off                       |                 |
| **C6**    | 101/1kV        | 100pF/1000V  | Unsolder or cut off                       |                 |
| **C10**   | 471/1kV        | 470pF/1000V  | Unsolder or cut off                       |                 |
| **C15**   | 22µF/35V       |              | Replace 2.2µF/35V                         |                 |
| **C16**   | 471/1kV        | 470pF/1000V  | Increase to 680pF : put 220pF in parallel |                 |

</div>

<div class="sect2">

### Resistors

| Component | Identification | Modification                                                        | Expected result |
|-----------|----------------|---------------------------------------------------------------------|-----------------|
| **R3**    | 2.2kΩ/1W       | Replace 1kΩ/1W or shunt with 2.2kΩ for 1.1kΩ                        |                 |
| **R5**    | 1MΩ/1W         | Shunt with 820kΩ for 450kΩ                                          |                 |
| **R6**    | 1MΩ/1W         | Shunt with 820k for 450kΩ                                           |                 |
| **R8**    | 2.2kΩ/1W       | Replace 1kΩ/1W or shunt with 2.2kΩ for 1.1kΩ                        |                 |
| **R15**   | 2.2kΩ/1W       | Replace 1.6kΩ/1W or shunt with 5.6kΩ for 1.58kΩ                     |                 |
| **R21**   | 2.2kΩ/1W       | Replace 1kΩ/1W or shunt with 2.2kΩ for 1.1kΩ                        |                 |
| **R23**   | 10kΩ/1W        | Replace 5.6kΩ/1W or shunt with 12kΩ for 5.45kΩ                      |                 |
| **R39**   | 220kΩ/1W       | Replace 1MΩ linear potentiometer                                    |                 |
| **R52**\* | 68Ω/25W        | Put in between 350V line and PCB. Mount against chassis for cooling |                 |
| **R53**\* | 68Ω/25W        | Put in between 350V line and PCB. Mount against chassis for cooling |                 |

</div>

</div>

</div>

<div class="sect1">

## Detailed findings

<div class="sectionbody">

<div class="sect2">

### Capacitors

<div class="sect3">

#### C3

<div class="ulist">

- **Stock value**: 470pF/1000V

- **Action**: unsolder or cut off

- **Affects**: Clean channel

- **Perceived result**:

  <div class="ulist">

  - Cuts brightness off

  - Clean channel sounds less treble-y. easier to manage

  </div>

</div>

</div>

<div class="sect3">

#### C4

<div class="ulist">

- **Stock value**: 470pF/1000V

- **Action**: unsolder or cut off

- **Affects**: Clean channel

- **Perceived result**: Cuts brightness off

</div>

</div>

<div class="sect3">

#### C5

<div class="ulist">

- **Stock value**: 470pF/400V

- **Action**: unsolder or cut off

- **Affects**: Both channels. Cuts brightness off

- **Perceived result**:

</div>

</div>

<div class="sect3">

#### C6

<div class="ulist">

- **Stock value**: 100pF/1000V

- **Action**: unsolder or cut off

- **Affects**: Both channels. Cuts brightness off

- **Perceived result**: Drive channel now sounds more like a fizzy
  distortion rather than a crunchy overdrive

</div>

</div>

<div class="sect3">

#### C10

<div class="ulist">

- **Stock value**: 470pF/1000V

- **Action**: unsolder or cut off

- **Affects**: Both channels. Cuts brightness off

- **Perceived result**: lost way too much top end

</div>

<div class="sect4">

##### C15

<div class="ulist">

- **Stock value**: 22uF/35V

- **Action**: replace 2.2uF/35V

- **Affects**: overdrive channel

- **Perceived result**: makes the whole amp brighter, less bottom end.

</div>

</div>

</div>

<div class="sect3">

#### C16

<div class="ulist">

- **Stock value**: 470pF/1000V

- **Action**: increase to 680pF, put capacitor in parallel

- **Affects**:

- **Perceived result**:

</div>

</div>

</div>

<div class="sect2">

### Resistors

<div class="sect3">

#### R3

<div class="ulist">

- **Stock value**: 2.2kΩ/1W

- **Action**: replace 1k/1W

- **Affects**: ?

- **Perceived result**:?

</div>

</div>

<div class="sect3">

#### R5

<div class="ulist">

- **Stock value**: 1MΩ/1W

- **Action**: shunt with 820kΩ/1W

- **Affects**: Drive channel

- **Perceived result**:

  <div class="ulist">

  - The fundamental pass frequency is halved

  - Potential divider is halved (more signal to next stage)

  - The sound opens up with increased bass but no loss of top end, not
    dark and not harsh

  </div>

</div>

</div>

<div class="sect3">

#### R6

<div class="ulist">

- **Stock value**: 1MΩ/1W

- **Action**: shunt with 820kΩ/1W

- **Affects**: Drive channel

- **Perceived result**:

  <div class="ulist">

  - The fundamental pass frequency is halved

  - Potential divider is halved (more signal to next stage)

  - The sound opens up with increased bass but no loss of top end, not
    dark and not harsh

  </div>

</div>

</div>

<div class="sect3">

#### R8

<div class="ulist">

- **Stock value**: 2.2kΩ/1W

- **Action**: replace 1kΩ/1W

- **Affects**: Drive channel

- **Perceived result**: ?

</div>

</div>

<div class="sect3">

#### R15

<div class="ulist">

- **Stock value**: 2.2kΩ/1W

- **Action**: replace 1.6kΩ/1W

- **Affects**: Drive channel

- **Perceived result**:

  <div class="ulist">

  - Smoother overdrive : affects overdrive channel

  - Lowered gain → ???

  - Distortion is a bit smoother and not so harsh

  - Really mellows out the channel drive

  - The drive is more manageable

  - Is the best sounding, most noticeable among the mods mentioned.

  - Unbelievable difference. That gain boost brought some fire into the
    tone. Gain was nicely saturated & balanced.

  </div>

</div>

</div>

<div class="sect3">

#### R23

<div class="ulist">

- **Stock value**: 10kΩ/1W

- **Action**: replace 5.6kΩ/1W

- **Affects**: ?

- **Perceived result**: ?

</div>

</div>

<div class="sect3">

#### R39

<div class="ulist">

- **Stock value**: 220kΩ/1W

- **Action**:

  <div class="ulist">

  - Replace by 1MΩ potentiometer

  - Place potentiometer in front panel of the chassis

  - Wire to PCB with shielded signal cable

  </div>

- **Affects**: Volume control improvement of clean channel

- **Perceived result**:

  <div class="ulist">

  - Better control of clean channel volume

  - Clean channel can be cranked up and controlled by master volume knob

  </div>

</div>

</div>

<div class="sect3">

#### R52

<div class="ulist">

- **Stock value**: not existing

- **Action**:

  <div class="ulist">

  - Put in between 350V AC lead of transformer and SS2 on the PCB

  </div>

- **Affects**: Protects against transformer blow-up

- **Perceived result**: None. Increases robustness of amplifier

</div>

</div>

<div class="sect3">

#### R53

<div class="ulist">

- **Stock value**: not existing

- **Action**:

  <div class="ulist">

  - Put in between 350V AC lead of transformer and SS1 on the PCB

  </div>

- **Affects**: Protects against transformer blow-up

- **Perceived result**: None. Increases robustness of amplifier

</div>

<div id="img" class="imageblock related thumb center">

<div class="content">

![rating chart III](rating-chart-III.png)

</div>

<div class="title">

Figure 1. Limiting resistance calculation graph

</div>

</div>

<div class="paragraph">

It is worth noting that the choke will slightly lower the required
limiting resistance, but not by much. The above datasheet chart provides
the calculation necessary to determine the limiting resistance provided
by the transformer:

</div>

<div class="paragraph">

Rs = Rsec + N2Rpri

</div>

<div class="paragraph">

measured Rpri is 5.5R and Rsec is 37.5R (75R for the entire winding) so…​

</div>

<div class="paragraph">

Transformer ratio N = 350 / 240 = 1.45 so…​

</div>

<div class="paragraph">

37.5 + (1.45 \* 1.45 \* 5.5) = 49R

</div>

<div class="paragraph">

The chart shows that for a 350V tap, the limiting resistance needs to be
around 105R for EACH PLATE of the rectifier, and this is for a fresh
valve manufactured to 1959 standards. So lets assume 115R will be safer
for modern 5AR4s,

</div>

<div class="paragraph">

115R - 49R = 66R

</div>

<div class="paragraph">

68R is the nearest standard resistor, two of these should be chassis
mounted inside the amp, and the transformer secondary taps (350V) should
be wired directly to these and then the other ends of the resistors
wired to the PCB. 25W types are recommended, as the voltage rating
should be in the region of 550V. The working voltage in practice will be
much much lower than this but at the moment of power on, it will be
higher and it’s nice to know that nothing can go wrong!

</div>

<div class="paragraph">

BTW, using the standby switch makes the problem worse because the
rectifier is fully ready to conduct. If you are unlucky enough to flip
the switch at the moment the mains AC voltage is at it’s peak then
destruction is almost guaranteed. Starting the amp from cold (without
the standby) will slightly reduce the likelihood of failure. So, for a
happy BC30, don’t use the standby and install limiting resistors!

</div>

<div id="img" class="imageblock related thumb center">

<div class="content">

![modded power supply 68R](modded-power-supply-68R.png)

</div>

<div class="title">

Figure 2. Limiting resistors of 68Ω

</div>

</div>

</div>

</div>

<div class="sect2">

### Tried out combinations and their findings

<div class="admonitionblock note">

<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr class="odd">
<td class="icon"><div class="title">
Note
</div></td>
<td class="content">C5,C6,C10,R15<br />
<strong>This amp is singing now<br />
</strong> Much more manageable/tweak-able</td>
</tr>
</tbody>
</table>

</div>

<div class="admonitionblock note">

<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr class="odd">
<td class="icon"><div class="title">
Note
</div></td>
<td class="content">C3,C5,C6,R15<br />
In that order you’ll hear the difference</td>
</tr>
</tbody>
</table>

</div>

<div class="admonitionblock note">

<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr class="odd">
<td class="icon"><div class="title">
Note
</div></td>
<td class="content">C3,C5,C6,C15,R15,R21<br />
<strong>These mods really open this amp up<br />
</strong> EQ is way more definable between active and not<br />
** More use out of the mid channel</td>
</tr>
</tbody>
</table>

</div>

<div class="admonitionblock note">

<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr class="odd">
<td class="icon"><div class="title">
Note
</div></td>
<td class="content">C3,C5,C6,R15,R21,C15<br />
<strong>The thing now sounds great<br />
</strong> EQ is extremely effective and sensitive<br />
<strong>Has more gain but it is by no means a 'high gain' amp<br />
</strong> Bigger sound and more responsive to touch<br />
</td>
</tr>
</tbody>
</table>

</div>

</div>

<div class="sect2">

### Other modifications

<div class="sect3">

#### Remove standby switch SW4

<div class="ulist">

- Use the whole for master volume mod

</div>

</div>

<div class="sect3">

#### "Fat Gain" switch

<div id="img" class="imageblock related thumb center">

<div class="content">

![fat gain switch](fat-gain-switch.jpg)

</div>

<div class="title">

Figure 3. Fat gain switch

</div>

</div>

<div class="ulist">

- Use a DPDT (Double Pole, Double Throw) switch

</div>

</div>

<div class="sect3">

#### Other recommendations

<div class="ulist">

- Tidy all wirings

- Re-valve Rectifier replacement

</div>

</div>

<div class="sect3">

#### Pre-amp Tube recommendations

<div class="ulist">

- Ei ECC83

- RCA

- Mullard

- Telefunken

- JJ ECC83

- **V1, V2**: Harma ECC83 Retro (Mullard rebuild)

- **V3, V5**: (reverb) Stock EH so far

</div>

</div>

<div class="sect3">

#### Phase inverter recommendations

<div class="ulist">

- **V4**: (PI) Harma ECC83 Retro - balanced

</div>

</div>

<div class="sect3">

#### Output valve recommendations

<div class="ulist">

- Tungsol 5881

- Svetlana 6L6

- JJ Electronic 5881

- Groove Tube 5881

- NOS Philips JAN 6L6WGB/5881

- Svetlana=C= 6L6GC

</div>

</div>

<div class="sect3">

#### Reverb drive valve recommendations

<div class="ulist">

- Sovtek 5AR4

</div>

</div>

</div>

<div class="sect2">

### Other hardware

<div class="sect3">

#### Output transformer and choke

<div class="ulist">

- Mercury Magnetics is selling a replacement output transformer and
  choke

</div>

</div>

</div>

</div>

</div>

<div class="sect1">

## Links

<div class="sectionbody">

<div class="ulist">

- [Mercury
  Magnetics](https://www.mercurymagnetics.com/products/?swoof=1&filter_make=epiphone&filter_model=blues-custom&filter_product-type=all)

- [Switch types](https://en.wikipedia.org/wiki/Switch)

- [forum
  discussion](https://www.tdpri.com/threads/anyone-try-the-new-epiphone-blues-custom.54114/)

</div>

</div>

</div>

</div>

<div id="footer">

<div id="footer-text">

Last updated 2024-11-03 21:01:34 +0100

</div>

</div>
