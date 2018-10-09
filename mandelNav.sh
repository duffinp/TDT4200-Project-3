#!/bin/bash

BIN=mandel/mandel
OUTDIR=output
OUT=${OUTDIR}/mandelnav.png

[ -f .mandelnav ] && {
    echo "Load previous session? [y/N]"
    while [ 1 -eq 1 ]; do
        read -n 1 -s -r C
        case $C in
            n|N)
                rm .mandelnav
                break
                ;;
            y|Y)
                break
                ;;
        esac
    done
}


[ -f .mandelnav ] && source .mandelnav || {
X=0.5
Y=0.5
MOVE=0.01
STEP=1.10
S=1.0
I=128
RES=800
cI=1
MARK=""
BLOCKDIM=16
TRADITIONAL=""
SUBDIV=4
}

[ ! -f ${BIN} ] && {
    echo "Could not find executable ${BIN}!"
    exit 1
}

which /usr/bin/time >/dev/null 2>&1 || {
    echo "Application '/usr/bin/time' is required";
    exit 1
}

which bc >/dev/null 2>&1 || {
    echo "Application 'bc' is required!"
    exit 1
}

which tput >/dev/null 2>&1 || {
    echo "Application 'tput' is required!"
    exit 1
}

mkdir -p $OUTDIR
printf "\rRender initial image..."
TIME=$(/usr/bin/time -f "%E" $BIN -q -r $RES -x $X -y $Y -s $S -i $I -c $cI -b $BLOCKDIM -d $SUBDIV -o $OUT $MARK $TRADITIONAL 2>&1 >/dev/null)
[ $? -ne 0 ] && {
    echo "Failed to render initial image!"
    exit 1
}

[ $# -gt 0 ] && {
    which "$1" >/dev/null 2>&1 || {
        echo "Application '$1' is required!"
        exit 1
    }
    $1 $OUT >/dev/null 2>&1 &
    trap "kill $! >/dev/null 2>&1" EXIT
}

EXEC=2
PAUSE=0

function help() {
    tput clear
    printf "Usage of MandelNav:\n\n"
    printf "[←] left               [→] right\n"
    printf "[↑] up                 [↓] down\n"
    printf "[+] increase zoom      [-] decrease zoom\n"
    printf "[R] resolution up      [r] resolution down\n"
    printf "[I] iteration up       [i] iteration down\n"
    printf "[Y] colours up         [y] colours down\n"
    printf "[U] block dim up       [u] block dim down\n"
    printf "[J] subdivision up     [j] subdivision down\n"
    printf "[M] increase move      [m] decrease move\n"
    printf "[S] step up            [s] step down\n"
    printf "[f] toogle mariani borders on/off\n"
    printf "[p] toogle rendering on/off\n"
    printf "[e] toggle mariani algorithm on/off\n"
    printf "[o] rerender\n\n"

    printf "Currently calling: %s\n" "$BIN -q -r $RES -x $X -y $Y -s $S -i $I -c $cI -b $BLOCKDIM -d $SUBDIV -o $OUT $MARK $TRADITIONAL"

    printf "\nPress any key to continue..."
    read -n 1 -s
}

while [ $EXEC -gt 0 ]; do
    tput clear
    printf "Center:         %s, %s\n" $X $Y
    printf "Zoom:           %s %%\n" $(echo 100/${S} | bc -l)
    printf "Iterations:     %llu\n" $I
    printf "Colours:        %llu\n" $cI
    printf "Block dim:      %llu\n" $BLOCKDIM
    printf "Subdivision:    %llu\n\n" $SUBDIV
    printf "Moving by:      %s\n" $MOVE
    printf "Step:           %s %%\n" $(echo "(($STEP - 1) * 100)" | bc -l)
    printf "Resolution:     %llux%llu\n\n" $RES $RES
    printf "Algorithm:      %s\n" $(echo $TRADITIONAL | grep -q '\-t' && echo "Traditional" || echo "Mariani-Silver")
    printf "Rendering time: %s\n\n" $TIME
    printf "[q] quit [h] help "
    [ $PAUSE -eq 1 ] && printf "[p] resume rendering" || printf "[p] pause rendering"

    EXEC=2
    read -n 1 -s -r C
    case $C in
        h|H)
            help
            ;;
        p|P)
           [ $PAUSE -eq 1 ] && PAUSE=0 || PAUSE=1
            EXEC=1
            ;;
        $'\x41')
            Y=$(echo "$Y - $MOVE" | bc -l)
            echo $Y'<'0 | bc -l | grep -q "1" && Y=0.0
            EXEC=1
            ;;
        $'\x42')
            Y=$(echo "$Y + $MOVE" | bc -l)
            echo $Y'>'1 | bc -l | grep -q "1" && Y=1.0
            EXEC=1
            ;;
        $'\x43')
            X=$(echo "$X + $MOVE" | bc -l)
            echo $X'>'1 | bc -l | grep -q "1" && X=1.0
            EXEC=1
            ;;
        $'\x44')
            X=$(echo "$X - $MOVE" | bc -l)
            echo $X'<'0 | bc -l | grep -q "1" && X=0.0
            EXEC=1
            ;;
        +)
            S=$(echo "$S / $STEP" | bc -l)
            MOVE=$(echo "$MOVE / $STEP" | bc -l)
            EXEC=1
            ;;
        -)
            S=$(echo "$S * $STEP" | bc -l)
            MOVE=$(echo "$MOVE * $STEP" | bc -l)
            echo $S'>'1 | bc -l | grep -q "1" && S=1.0
            EXEC=1
            ;;
        r)
            RES=$(echo "$RES - 100" | bc -l)
            [ $RES -lt 100 ] && RES=100
            EXEC=1
            ;;
        R)
            RES=$(echo "$RES + 100" | bc -l)
            EXEC=1
            ;;
        i)
            I=$(echo "$I - 4" | bc -l)
            [ $I -lt 4 ] && I=4
            EXEC=1
            ;;
        I)
            I=$(echo "$I + 4" | bc -l)
            EXEC=1
            ;;
        s)
            STEP=$(echo "$STEP - 0.01" | bc -l)
            echo $STEP'<='1 | bc -l | grep -q "1" && STEP=1.01
            ;;
        S)
            STEP=$(echo "$STEP + 0.01" | bc -l)
            ;;
        y)
            cI=$(echo "$cI - 1" | bc -l)
            echo $cI'<='1 | bc -l | grep -q "1" && cI=1
            EXEC=1
            ;;
        Y)
            cI=$(echo "$cI + 1" | bc -l)
            EXEC=1
            ;;
        u)
            BLOCKDIM=$(echo "$BLOCKDIM - 2" | bc -l)
            echo $BLOCKDIM'<='4 | bc -l | grep -q "1" && BLOCKDIM=4
            EXEC=1
            ;;
        U)
            BLOCKDIM=$(echo "$BLOCKDIM + 2" | bc -l)
            EXEC=1
            ;;
        j)
            SUBDIV=$(echo "$SUBDIV - 1" | bc -l)
            echo $SUBDIV'<='2 | bc -l | grep -q "1" && SUBDIV=2
            EXEC=1
            ;;
        J)
            SUBDIV=$(echo "$SUBDIV + 1" | bc -l)
            EXEC=1
            ;;
        m)
            MOVE=$(echo "$MOVE / $STEP" | bc -l)
            ;;
        M)
            MOVE=$(echo "$MOVE * $STEP" | bc -l)
            ;;
        t|T)
            [ "$MARK" == "-m" ] && MARK="" || MARK="-m"
            EXEC=1
            ;;
        e|E)
            [ "$TRADITIONAL" == "-t" ] && TRADITIONAL="" || TRADITIONAL="-t"
            EXEC=1
            ;;
        o|O)
            EXEC=1
            ;;
        q|Q)
            EXEC=0
            ;;
        *)
            EXEC=2
            ;;
    esac

    [ $EXEC -eq 1 ] && [ $PAUSE -eq 0 ] && {
        printf "\rRender new image...                   "
        TIME=$(/usr/bin/time -f "%E" $BIN -q -r $RES -x $X -y $Y -s $S -i $I -c $cI -b $BLOCKDIM -d $SUBDIV -o $OUT $MARK $TRADITIONAL 2>&1 >/dev/null)
        [ $? -ne 0 ] && {
            echo "Failed rendering image!"
            exit 1
        }
    }
done

echo "X=$X
Y=$Y
MOVE=$MOVE
STEP=$STEP
S=$S
I=$I
cI=$cI
MARK=$MARK
BLOCKDIM=$BLOCKDIM
SUBDIV=$SUBDIV
RES=$RES" > .mandelnav


