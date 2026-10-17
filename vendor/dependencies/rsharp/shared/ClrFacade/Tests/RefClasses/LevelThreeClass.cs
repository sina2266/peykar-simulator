namespace ClrFacade.Tests.RefClasses
{
    public class LevelThreeClass : LevelTwoClass
    {
        public override string AbstractMethod()
        {
            return "LevelThreeClass::AbstractMethod()";
        }

        public override string VirtualMethod()
        {
            return "LevelThreeClass::VirtualMethod()";
        }

        public override string IfBaseOneString
        {
            get
            {
                return base.IfBaseOneString;
            }
            set
            {
                base.IfBaseOneString = "Overriden LevelThreeClass::IfBaseOneString " + value;
            }
        }
    }
}
