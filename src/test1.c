/* Test program for the C-minus compiler
 * Recieves input of five integers
 * and prints the min and max values */

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
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
    int maximum;
    int minimum;

    read(x, 5);

    maximum = 0;
    minimum = 2147483647;
    i = 0;
    while (i < 5)
    {
        minimum = min(minimum, x[i]);
        maximum = max(maximum, x[i]);
        i = i + 1;
    }
    output(minimum);
    output(maximum);
}