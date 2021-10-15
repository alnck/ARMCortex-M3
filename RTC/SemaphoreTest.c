s1 = CreateSemaphore();
m1 = CreateMutex();

// preemptive

B() 
{
  WaitSignal(s1);
  
  printf("...");
  
  
}


A()
{
  ...
  Signal(s1);
  ...
}

// Mutex: Mutual Exclusion

A()
{
  WaitMutex(m1);
  
  TakeMutex(m1);
  ....
  // Kayanaðý kullanýyoruz
  ...
  GiveMutex();  // Free
}

B()
{
  WaitMutex(m1);
  
  TakeMutex(m1);
  ....
  // Kayanaðý kullanýyoruz
  ...
  GiveMutex();  // Free
}

C()
{
  WaitMutex(m1);
  
  TakeMutex(m1);
  ....
  // Kayanaðý kullanýyoruz
  ...
  GiveMutex();  // Free
}

// cooperative

A()
{
  ...
  switch (state) {
  case S_WAITSEM:
    if (g_Sem1) {
      g_Sem1 = 0;
      state = S_NEXT;
    }
    break;
    
  case S_NEXT:
    ...
  }
}