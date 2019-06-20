/* Test program for the C-minus compiler
 * Recieves input of five integers
 * and prints the number of even numbers */

int mod(int a, int b)
{
    return a - a / b * b;
}

void read(int a[], int n)
{
    int i;
    i = 0;
    while (i < n)
    {
        a[i] = input();
        i = i + 1;
    }
}

void main(void)
{
    int x[5];
    int i;
    int count;

    read(x, 5);

    count = 0;
    i = 0;
    while (i < 5)
    {
        if (mod(x[i], 2) == 0)
        {
            count = count + 1;
        }
        i = i + 1;
    }
    output(count);
}