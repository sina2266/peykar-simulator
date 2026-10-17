using System;
using System.Diagnostics;
using System.Dynamic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using RDotNet.Dynamic;
using RDotNet.Internals;
using RDotNet.Utilities;

namespace RDotNet
{
   /// <summary>
   ///    An expression in R environment.
   /// </summary>
   [DebuggerDisplay("RObjectType = {Type}")]
   [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
   public class SymbolicExpression : SafeHandle, IEquatable<SymbolicExpression>, IDynamicMetaObjectProvider
   {
      private readonly dynamic sexp;

      /// <summary>
      ///    An object to use to get a lock on if EnableLock is true;
      /// </summary>
      /// <remarks>
      ///    Following recommended practices in http://msdn.microsoft.com/en-us/library/vstudio/c5kehkcz(v=vs.120).aspx
      /// </remarks>
      private static readonly Object lockObject = new Object();

      /// <summary>
      ///    Managed id of the R main thread. R's memory manager is single threaded, so
      ///    R_PreserveObject / R_ReleaseObject must only run on this thread. Captured on the
      ///    first Preserve() (always called on the R main thread) so Unpreserve() can detect
      ///    when it is on the GC finalizer thread and avoid calling into R from there.
      /// </summary>
      private static int _rThreadId;

      /// <summary>
      ///    Creates new instance of SymbolicExpression.
      /// </summary>
      /// <param name="engine">The engine.</param>
      /// <param name="pointer">The pointer.</param>
      protected internal SymbolicExpression(REngine engine, IntPtr pointer)
         : base(IntPtr.Zero, true)
      {
         this.Engine = engine;
         var sexprecType = engine.GetSEXPRECType();
         this.sexp = Convert.ChangeType(Marshal.PtrToStructure(pointer, sexprecType), sexprecType);
         SetHandle(pointer);
         Preserve();
      }

      /// <summary>
      ///    Is the handle of this SEXP invalid (zero, i.e. null pointer)
      /// </summary>
      public override bool IsInvalid
      {
         get { return handle == IntPtr.Zero; }
      }

      /// <summary>
      ///    Gets the <see cref="REngine" /> to which this expression belongs.
      /// </summary>
      public REngine Engine { get; }

      /// <summary>
      ///    Creates the delegate function for the specified function defined in the DLL.
      /// </summary>
      /// <typeparam name="TDelegate">The type of delegate.</typeparam>
      /// <returns>The delegate.</returns>
      protected internal TDelegate GetFunction<TDelegate>() where TDelegate : class
      {
         return Engine.GetFunction<TDelegate>();
      }

      /// <summary>
      ///    Gets whether this expression is protected from the garbage collection.
      /// </summary>
      public bool IsProtected { get; private set; }

      /// <summary>
      ///    Gets the <see cref="SymbolicExpressionType" />.
      /// </summary>
      public SymbolicExpressionType Type
      {
         get { return this.sexp.sxpinfo.type; }
      }

      #region IDynamicMetaObjectProvider Members

      /// <summary>
      ///    returns a new SymbolicExpressionDynamicMeta for this SEXP
      /// </summary>
      /// <param name="parameter"></param>
      /// <returns></returns>
      public virtual DynamicMetaObject GetMetaObject(System.Linq.Expressions.Expression parameter)
      {
         return new SymbolicExpressionDynamicMeta(parameter, this);
      }

      #endregion IDynamicMetaObjectProvider Members

      #region IEquatable<SymbolicExpression> Members

      /// <summary>
      ///    Testing the equality of SEXP, based on handle equality.
      /// </summary>
      /// <param name="other">other SEXP</param>
      /// <returns>True if the objects have a handle that is the same, i.e. pointing to the same address in unmanaged memory</returns>
      public bool Equals(SymbolicExpression other)
      {
         return other != null && handle == other.handle;
      }

      #endregion IEquatable<SymbolicExpression> Members

      internal object GetInternalStructure()
      {
         var sexprecType = Engine.GetSEXPRECType();
         return Convert.ChangeType(Marshal.PtrToStructure(handle, sexprecType), sexprecType);
      }

      /// <summary>
      ///    Gets all value names.
      /// </summary>
      /// <returns>The names of attributes</returns>
      public string[] GetAttributeNames()
      {
         int length = this.GetFunction<Rf_length>()(this.sexp.attrib);
         var names = new string[length];
         IntPtr pointer = this.sexp.attrib;
         var sexprecType = Engine.GetSEXPRECType();
         for (int index = 0; index < length; index++)
         {
            dynamic node = Convert.ChangeType(Marshal.PtrToStructure(pointer, sexprecType), sexprecType);
            var attribute = Convert.ChangeType(Marshal.PtrToStructure(node.listsxp.tagval, sexprecType), sexprecType);
            IntPtr name = attribute.symsxp.pname;
            names[index] = new InternalString(Engine, name);
            pointer = node.listsxp.cdrval;
         }

         return names;
      }

      /// <summary>
      ///    Gets the value of the specified name.
      /// </summary>
      /// <param name="attributeName">The name of attribute.</param>
      /// <returns>The attribute.</returns>
      public SymbolicExpression GetAttribute(string attributeName)
      {
         if (attributeName == null)
         {
            throw new ArgumentNullException();
         }

         if (attributeName == string.Empty)
         {
            throw new ArgumentException();
         }

         IntPtr installedName = this.GetFunction<Rf_install>()(attributeName);
         IntPtr attribute = this.GetFunction<Rf_getAttrib>()(handle, installedName);
         if (Engine.EqualsRNilValue(attribute))
         {
            return null;
         }

         return new SymbolicExpression(Engine, attribute);
      }

      internal SymbolicExpression GetAttribute(SymbolicExpression symbol)
      {
         if (symbol == null)
         {
            throw new ArgumentNullException();
         }

         if (symbol.Type != SymbolicExpressionType.Symbol)
         {
            throw new ArgumentException();
         }

         IntPtr attribute = this.GetFunction<Rf_getAttrib>()(handle, symbol.handle);
         if (Engine.EqualsRNilValue(attribute))
         {
            return null;
         }

         return new SymbolicExpression(Engine, attribute);
      }

      /// <summary>
      ///    Sets the new value to the attribute of the specified name.
      /// </summary>
      /// <param name="attributeName">The name of attribute.</param>
      /// <param name="value">The value</param>
      public void SetAttribute(string attributeName, SymbolicExpression value)
      {
         if (attributeName == null)
         {
            throw new ArgumentNullException();
         }

         if (attributeName == string.Empty)
         {
            throw new ArgumentException();
         }

         if (value == null)
         {
            value = Engine.NilValue;
         }

         IntPtr installedName = this.GetFunction<Rf_install>()(attributeName);
         this.GetFunction<Rf_setAttrib>()(handle, installedName, value.handle);
      }

      internal void SetAttribute(SymbolicExpression symbol, SymbolicExpression value)
      {
         if (symbol == null)
         {
            throw new ArgumentNullException();
         }

         if (symbol.Type != SymbolicExpressionType.Symbol)
         {
            throw new ArgumentException();
         }

         if (value == null)
         {
            value = Engine.NilValue;
         }

         this.GetFunction<Rf_setAttrib>()(handle, symbol.handle, value.handle);
      }

      /// <summary>
      ///    Protects the expression from R's garbage collector.
      /// </summary>
      /// <seealso cref="Unpreserve" />
      public void Preserve()
      {
         if (!IsInvalid && !IsProtected)
         {
            if (_rThreadId == 0)
            {
               _rThreadId = Environment.CurrentManagedThreadId;
               // Hand DeferredRelease the release mechanism, captured here on the R main
               // thread, so it can release (on this thread) handles that off-thread SafeHandle
               // finalizers queue. See Unpreserve and DeferredRelease.
               var engine = Engine;
               var releaseObject = this.GetFunction<R_ReleaseObject>();
               DeferredRelease.Configure(pendingHandle =>
               {
                  if (engine.EnableLock)
                     lock (lockObject) { releaseObject(pendingHandle); }
                  else
                     releaseObject(pendingHandle);
               });
            }
            if (Engine.EnableLock)
            {
               lock (lockObject)
               {
                  this.GetFunction<R_PreserveObject>()(handle);
               }
            }
            else
               this.GetFunction<R_PreserveObject>()(handle);

            this.IsProtected = true;
         }
      }

      /// <summary>
      ///    Stops protection.
      /// </summary>
      /// <seealso cref="Preserve" />
      public void Unpreserve()
      {
         if (!IsInvalid && IsProtected)
         {
            // Never call R_ReleaseObject off the R main thread. SafeHandle finalization runs
            // on the CLR finalizer thread; calling into R's single-threaded, non-reentrant
            // memory manager from there corrupts R's heap (a Windows access violation or a
            // Linux hang). When not on the R main thread (i.e. this is a finalizer), hand the
            // handle to DeferredRelease for the R main thread to release later, rather than
            // either crashing (releasing here) or leaking the preservation (skipping it).
            if (_rThreadId != 0 && Environment.CurrentManagedThreadId != _rThreadId)
            {
               DeferredRelease.Enqueue(handle);
               IsProtected = false;
               return;
            }

            if (Engine.EnableLock)
            {
               lock (lockObject)
               {
                  this.GetFunction<R_ReleaseObject>()(handle);
                  ;
               }
            }
            else
               this.GetFunction<R_ReleaseObject>()(handle);

            this.IsProtected = false;
         }
      }

      /// <summary>
      ///    Release the handle on the symbolic expression, i.e. tells R to decrement the reference count to the expression in
      ///    unmanaged memory
      /// </summary>
      /// <returns></returns>
      protected override bool ReleaseHandle()
      {
         if (IsProtected)
         {
            Unpreserve();
         }

         return true;
      }

      /// <summary>
      ///    Returns the hash code for this instance.
      /// </summary>
      /// <returns>Hash code</returns>
      public override int GetHashCode()
      {
         return handle.GetHashCode();
      }

      /// <summary>
      ///    Test the equality of this object with another. If this object is also a SymbolicExpression and points to the same R
      ///    expression, returns true.
      /// </summary>
      /// <param name="obj">Other object to test for equality</param>
      /// <returns>Returns true if pointing to the same R expression in memory.</returns>
      public override bool Equals(object obj)
      {
         return Equals(obj as SymbolicExpression);
      }

      /// <summary>
      ///    Experimental
      /// </summary>
      /// <typeparam name="K"></typeparam>
      /// <param name="sexp"></param>
      /// <param name="name"></param>
      /// <returns></returns>
      public static SymbolicExpression op_Dynamic<K>(SymbolicExpression sexp, string name)
      {
         throw new NotImplementedException();
      }

      /// <summary>
      ///    Experimental
      /// </summary>
      /// <typeparam name="K"></typeparam>
      /// <param name="sexp"></param>
      /// <param name="name"></param>
      /// <param name="value"></param>
      public static void op_DynamicAssignment<K>(SymbolicExpression sexp, string name, dynamic value)
      {
      }
   }
}