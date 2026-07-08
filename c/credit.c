#include <stdio.h>

void split_n(long long n, int first[], int *first_len, int second[], int *second_len);
void check_n(int starting_d, int length, const int first[], int first_len, const int second[], int second_len);
int get_starting(long long n, int *length);

int main(void)
{
    int length = 0;
    int first[20]; 
    int second[20];
    int f_len = 0;
    int s_len = 0;

    long long n;
    while (1)
    {
        printf("Number: ");

        if (scanf("%lld", &n) == 1 && n >= 0)
            break;

        while (getchar() != '\n');
    }

    int starting_d = get_starting(n, &length);

    split_n(n, first, &f_len, second, &s_len);
    check_n(starting_d, length, first, f_len, second, s_len);
}

void split_n(long long n, int first[], int *first_len, int second[], int *second_len)
{
    int i1 = 0;
    int i2 = 0;
    while (n > 0)
    {
        first[i1++] = n % 10;
        n /= 10;

        if (n == 0) break;

        second[i2++] = n % 10;
        n /= 10;
    }
    *first_len = i1;
    *second_len = i2;
}

void check_n(int starting_d, int length, const int first[], int first_len, const int second[], int second_len)
{
    int sum = 0;

    for(int x = 0; x < first_len; x++)
    {
        sum += first[x];
    }

    for(int y = 0; y < second_len; y++)
    {
        int digit = second[y] * 2;
        if(digit > 9)
        {
            digit -= 9;
            sum += digit;
        }
        else
        {
            sum += digit;
        }
    }
    
    if(sum % 10 == 0)
    {
        int visa_condition = (starting_d >= 10) ? (starting_d / 10 == 4) : (starting_d == 4);

        if(visa_condition && (length == 13 || length == 16))
        {
            printf("VISA\n");
        }
        else if((starting_d == 34 || starting_d == 37) && length == 15)
        {
            printf("AMEX\n");
        }
        else if((starting_d >= 51 && starting_d <= 55) && length == 16)
        {
            printf("MASTERCARD\n");
        }
        else
        {
            printf("INVALID\n");
        }
    }
    else
    {
        printf("INVALID\n");
    }
}

int get_starting(long long n, int *length)
{
    int len = 0;
    for (long long t = n; t > 0; t /= 10) len++;
    *length = len;

    while (n >= 100) n /= 10;
    return (int)n;
}
