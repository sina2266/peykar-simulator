namespace ClrFacade.Tests.RefClasses
{
    public interface InterfaceOne
    {
        string IfOneString { get; set; }
        string IfOneStringGetter { get; }
    }

    public interface InterfaceBaseTwo
    {
        string IfBaseTwoString { get; set; }
        string IfBaseTwoMethod();
        string IfBaseTwoMethod(string par);
    }

    public interface InterfaceBaseOne
    {
        string IfBaseOneString { get; set; }
    }

    public interface InterfaceTwo : InterfaceBaseOne, InterfaceBaseTwo
    {
    }
}
