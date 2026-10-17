namespace ClrFacade.Tests.RefClasses
{
    public abstract class BaseAbstractClassOne
    {
        protected BaseAbstractClassOne(int someInt)
        {
            this.SomeInt = someInt;
        }
        protected BaseAbstractClassOne()
        {
        }

        public int SomeInt { get; private set; }

        public abstract string AbstractMethod();
        public abstract string AbstractMethod(string arg);

        public virtual string VirtualMethod() { return "BaseAbstractClassOne.VirtualMethod"; }

    }
}
