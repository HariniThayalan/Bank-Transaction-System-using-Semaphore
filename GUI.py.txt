import tkinter as tk
from tkinter import messagebox
import subprocess
import threading

def generate_bill(details):

    bill_window = tk.Toplevel(root)
    bill_window.title("Transaction Receipt")
    bill_window.geometry("400x300")


    receipt_text = f"""
***********************************
{details}
***********************************
Thank you for your banking service
"""
    tk.Label(bill_window, text=receipt_text, font=("Courier", 12), wraplength=350, justify="left").pack(pady=20)
    tk.Button(bill_window, text="Exit", bg="red", fg="white", command=bill_window.destroy).pack(pady=20)


def handle_transaction(pin, operation, amount):
    try:

        sender_process = subprocess.Popen(
            ['./Sender'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        input_data = f"{pin}\n{operation}\n{amount}\n"
        _, error = sender_process.communicate(input=input_data)

        if sender_process.returncode != 0 or error:
            messagebox.showerror("Transaction Failed", f"Error in sender program: {error.strip()}", parent=root)
            return


        receiver_process = subprocess.Popen(
            ['./Receiver'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
        )
        result, error = receiver_process.communicate()

        if receiver_process.returncode != 0 or error:
            messagebox.showerror("Transaction Failed", f"Error in receiver program: {error.strip()}", parent=root)
            return


        if "Error:" in result:
            messagebox.showerror("Operation Failed", result.strip(), parent=root)
        else:
            generate_bill(result.strip())

    except Exception as e:
        messagebox.showerror("Transaction Failed", f"Error: {e}", parent=root)

def create_new_account():
    def submit():
        pin = pin_entry.get()
        bal = balance_entry.get()
        if not (pin and bal.isdigit()):
            messagebox.showerror("Invalid Input", "Enter a valid PIN and balance", parent=win)
            return
        if int(bal) < 1000:
            messagebox.showerror("Minimum Balance Error", "Initial balance must be at least 1000", parent=win)
            return
        handle_transaction(pin, "Create", bal)
        win.destroy()

    win = tk.Toplevel(root)
    win.title("Create Account")
    win.geometry("300x200")
    tk.Label(win, text="Enter New PIN:").pack(pady=5)
    pin_entry = tk.Entry(win)
    pin_entry.pack(pady=5)
    tk.Label(win, text="Initial Balance:").pack(pady=5)
    balance_entry = tk.Entry(win)
    balance_entry.pack(pady=5)
    tk.Button(win, text="Submit", command=submit).pack(pady=10)
def simulate_deadlock_window():

    sim_window1 = tk.Toplevel(root)
    sim_window1.title("Transaction 1")
    sim_window1.geometry("400x300")

    tk.Label(sim_window1, text="Enter PIN 1:").pack(pady=5)
    pin_entry1 = tk.Entry(sim_window1)
    pin_entry1.pack(pady=5)

    tk.Label(sim_window1, text="Enter Operation 1 (Deposit/Withdraw):").pack(pady=5)
    operation_entry1 = tk.Entry(sim_window1)
    operation_entry1.pack(pady=5)

    tk.Label(sim_window1, text="Enter Amount 1:").pack(pady=5)
    amount_entry1 = tk.Entry(sim_window1)
    amount_entry1.pack(pady=5)


    sim_window2 = tk.Toplevel(root)
    sim_window2.title("Transaction 2")
    sim_window2.geometry("400x300")

    tk.Label(sim_window2, text="Enter PIN 2:").pack(pady=5)
    pin_entry2 = tk.Entry(sim_window2)
    pin_entry2.pack(pady=5)

    tk.Label(sim_window2, text="Enter Operation 2 (Deposit/Withdraw):").pack(pady=5)
    operation_entry2 = tk.Entry(sim_window2)
    operation_entry2.pack(pady=5)

    tk.Label(sim_window2, text="Enter Amount 2:").pack(pady=5)
    amount_entry2 = tk.Entry(sim_window2)
    amount_entry2.pack(pady=5)

    def simulate_deadlock():
        pin1 = pin_entry1.get()
        pin2 = pin_entry2.get()
        operation1 = operation_entry1.get()
        operation2 = operation_entry2.get()
        amount1 = amount_entry1.get()
        amount2 = amount_entry2.get()

        if not pin1 or not pin2 or not operation1 or not operation2 or not amount1.isdigit() or not amount2.isdigit():
            messagebox.showerror("Invalid Input", "Please ensure all fields are properly filled out.", parent=root)
            return

        if pin1 == pin2:
            messagebox.showinfo("Deadlock Simulation", "Deadlock Detected! Transaction 1 will execute first and Transaction 2 will wait.", parent=root)

            def transaction2():
                handle_transaction(pin2, operation2, amount2)

            handle_transaction(pin1, operation1, amount1)
            transaction2()

        else:
            handle_transaction(pin1, operation1, amount1)
            handle_transaction(pin2, operation2, amount2)


    tk.Button(sim_window2, text="Simulate", command=simulate_deadlock).pack(pady=10)

def transaction_window(operation):
    def submit():
        pin = pin_entry.get()
        amt = amount_entry.get() if operation != "Check Balance" else "0"
        if not pin or (operation != "Check Balance" and not amt.isdigit()):
            messagebox.showerror("Error", "Invalid input!", parent=win)
            return
        if operation == "Withdraw" and int(amt) <= 0:
            messagebox.showerror("Error", "Amount must be greater than 0 for withdrawal!", parent=win)
            return
        handle_transaction(pin, operation, amt)
        win.destroy()

    win = tk.Toplevel(root)
    win.title(f"{operation} Operation")
    win.geometry("300x200")
    tk.Label(win, text="Enter PIN:").pack(pady=5)
    pin_entry = tk.Entry(win)
    pin_entry.pack(pady=5)
    amount_entry = None
    if operation in ["Deposit", "Withdraw"]:
        tk.Label(win, text="Enter Amount:").pack(pady=5)
        amount_entry = tk.Entry(win)
        amount_entry.pack(pady=5)
    tk.Button(win, text="Submit", command=submit).pack(pady=10)

def setup_main_window():
    tk.Label(root, text="Welcome to the ATM System", font=("Arial", 18)).pack(pady=20)
    tk.Button(root, text="Create New Account", command=create_new_account).pack(pady=10)
    tk.Button(root, text="Deposit", command=lambda: transaction_window("Deposit")).pack(pady=10)
    tk.Button(root, text="Withdraw", command=lambda: transaction_window("Withdraw")).pack(pady=10)
    tk.Button(root, text="Check Balance", command=lambda: transaction_window("Check Balance")).pack(pady=10)

    tk.Button(root, text="Simulate Deadlock", command=simulate_deadlock_window).pack(pady=10)

    tk.Button(root, text="Exit", bg="red", fg="white", command=on_exit).pack(pady=20)


def on_exit():
    print("Exiting the application...")
    root.destroy()

root = tk.Tk()
root.title("ATM System")
root.geometry("400x400")
setup_main_window()
root.mainloop()

