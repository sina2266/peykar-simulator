using Microsoft.Win32;

namespace RDotNet.NativeLibrary
{
   /// <summary> Interface for registry keys.</summary>
   public interface IRegistryKey
   {
      /// <summary> Gets sub key names.</summary>
      /// <returns> An array of string.</returns>
      string[] GetSubKeyNames();

      /// <summary> Gets a value of a key-value pair</summary>
      /// <param name="name"> The name.</param>
      /// <returns> The value.</returns>
      object GetValue(string name);

      /// <summary>
      ///    Retrieves an array of strings that contains all the value names associated with
      ///    this key
      /// </summary>
      /// <returns> An array of string.</returns>
      string[] GetValueNames();

      /// <summary> Opens sub key.</summary>
      /// <param name="name"> The name.</param>
      /// <returns> An IRegistryKey.</returns>
      IRegistryKey OpenSubKey(string name);
   }

   /// <summary> Interface for registry.</summary>
   public interface IRegistry
   {
      /// <summary> Gets the local machine.</summary>
      /// <value> The local machine.</value>
      IRegistryKey LocalMachine { get; }

      /// <summary> Gets the current user.</summary>
      /// <value> The current user.</value>
      IRegistryKey CurrentUser { get; }
   }

   /// <summary> The windows registry.</summary>
   public class WindowsRegistry : IRegistry
   {
      /// <summary> Gets the current user.</summary>
      /// <value> The current user.</value>
      public IRegistryKey CurrentUser
      {
         get { return new WindowsRegistryKey(Registry.CurrentUser); }
      }

      /// <summary> Gets the local machine.</summary>
      /// <value> The local machine.</value>
      public IRegistryKey LocalMachine
      {
         get { return new WindowsRegistryKey(Registry.LocalMachine); }
      }
   }

   /// <summary> The windows registry key.</summary>
   public class WindowsRegistryKey : IRegistryKey
   {
      /// <summary> Constructor.</summary>
      /// <param name="realKey"> The real key.</param>
      public WindowsRegistryKey(RegistryKey realKey)
      {
         this.realKey = realKey;
      }

      readonly RegistryKey realKey;

      /// <summary> Gets sub key names.</summary>
      /// <returns> An array of string.</returns>
      public string[] GetSubKeyNames()
      {
         return realKey.GetSubKeyNames();
      }

      /// <summary> Gets a value of a key-value pair.</summary>
      /// <param name="name"> The name.</param>
      /// <returns> The value.</returns>
      public object GetValue(string name)
      {
         return realKey.GetValue(name);
      }

      /// <summary>
      ///    Retrieves an array of strings that contains all the value names associated with
      ///    this key.
      /// </summary>
      /// <returns> An array of string.</returns>
      public string[] GetValueNames()
      {
         return realKey.GetValueNames();
      }

      /// <summary> Opens sub key.</summary>
      /// <param name="name"> The name.</param>
      /// <returns> An IRegistryKey.</returns>
      public IRegistryKey OpenSubKey(string name)
      {
         return new WindowsRegistryKey(realKey.OpenSubKey(name));
      }
   }
}