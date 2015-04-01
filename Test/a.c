int a[10];
int temp;
int i;
int j;
main()
{
	i = 0;
	while( i < 10 )
	{
		read( a[i] );
		i = i + 1;
	}
	i = 0; 
	while( i < 10 )
	{
		j = i;
		while( j < 10 )
		{
			if( a[i] > a[j] )
			{
				temp = a[i];
				a[i] = a[j];
				a[j] = temp;
			}
			j = j + 1;
		}
	}
	i = 0;
	while( i < 10 )
	{
		write( a[i] );
		i = i + 1;
	}
}