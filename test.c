int a[10];
int i;
int j;
int temp;
main()
{
    a[0] = 8;
    a[1] = 2;
    a[2] = 3;
    a[3] = 7;
    i = 0;
    while( i < 4 )
    {
        j = i;
        while( j < 4 )
        {
            if( a[i] < a[j] )
            {
                temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
            j = j+1;
        }
        i = i + 1;
    }
    i = 0;
    while( i < 4 )
    {
        write( a[i] );
        i = i + 1;
    }
}
